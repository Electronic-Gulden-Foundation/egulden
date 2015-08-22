// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,      uint256("0x6d39f28ad01a7edd3e2374b355cf8c7f8dbc1c5e4596ad3642fa6d10c2599217"))
        ( 25000,  uint256("0x99ffd2f7230224586c61deb459fa1cde94bda2876aed8bb791f4e3b5d485ef59"))
        ( 40000,  uint256("0xbec732146705189352bb0007c4a26345a3ddf0eac1e8721e2fbb121b7d7c04b6"))
        ( 50000,  uint256("0x08ac154d59f223c3c6912a03ba44487bedabd27147e0da05a7aa339fcc84e011"))
        ( 75000,  uint256("0xd5e057b7a45fe9eb72326b5e7d7caaf31e0fd8f016425e41d8f88c9c196417c5"))
        ( 100000, uint256("0xde49a9254ea2f01287a8b2f9d97438a52809cfd78d3c5bcc13fa529a07989083"))
        ( 125000, uint256("0x8f817d3fdeb7fbb338d29dc8587a127c34d98617acadfd4248e4f9dfd23fd672"))
        ( 150000, uint256("0x14eac37f76d8be510df10526e3ff053aa472d7f2e507ea3562a74b60e7174961"))
        ( 175000, uint256("0x9d7d7db04e9dc15aa06abb45346b9a96a9136c76b6d52465eea5c3f01f861ad7"))
        ( 200000, uint256("0x856d56f908ecde89d24ed216978f5b9898000defc15f8dcfac83a3a6eaeea9b9"))
        ( 225000, uint256("0x1ab99f5f8ea110de1a57e128e883b760152dba409c2c1130353edcea67643277"))
        ( 250000, uint256("0x0086da045fd362f346d1e7dcc12fe41e7eca4dc6bcd2515bdec7b7a1383c0ec9"))
        ( 300000, uint256("0x33c0d8ea44fefa9ade4aee0032e0784e7e58ecac562129a7c0a5904472745863"))
        ( 350000, uint256("0x1235b66a56c1c01c51902fd9bf7849e6bdf859ea2e21befa6a04cfcf0f398acf"))
        ( 400000, uint256("0x77e2db7ef4a5704a21ae5768405f695db17be807fef7989ce16fac07eaeb5413"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1421000263, // * UNIX timestamp of last checkpoint block
        92000,      // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        150.0       // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (   546, uint256("0xa0fea99a6897f531600c8ae53367b126824fd6a847b2b2b73817a95b8e27e602"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1365458829,
        547,
        576
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
