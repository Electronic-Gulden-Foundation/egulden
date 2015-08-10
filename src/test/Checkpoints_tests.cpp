// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(Checkpoints_tests)

BOOST_AUTO_TEST_CASE(sanity)
{
    uint256 p25000 = uint256("0x99ffd2f7230224586c61deb459fa1cde94bda2876aed8bb791f4e3b5d485ef59");
    uint256 p40000 = uint256("0xbec732146705189352bb0007c4a26345a3ddf0eac1e8721e2fbb121b7d7c04b6");
    BOOST_CHECK(Checkpoints::CheckBlock(25000, p25000));
    BOOST_CHECK(Checkpoints::CheckBlock(40000, p40000));


    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(25000, p40000));
    BOOST_CHECK(!Checkpoints::CheckBlock(40000, p25000));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(25000+1, p40000));
    BOOST_CHECK(Checkpoints::CheckBlock(40000+1, p25000));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 300000);
}

BOOST_AUTO_TEST_SUITE_END()
