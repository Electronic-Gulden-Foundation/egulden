// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of the genesis coinbase cannot
 * be spent as it did not originally exist in the database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Mtgox is down";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";

        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 710000;
        consensus.BIP34Hash = uint256S("fa09d204a83a768ed5a7c8d441fa62f2043abf420cff1226c7b4329aeb9d51cf");
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 270000; // 3.5 days
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1485561600; // January 28, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1485561600; // January 28, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000017336a6a46bff5d");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        /**
         * TODO: Enforce this pchMessageStart, will cause newer clients to not connect with the old anymore
        pchMessageStart[0] = 0x80; // €
        pchMessageStart[1] = 0x83; // ƒ
        pchMessageStart[2] = 0x4c; // L
        pchMessageStart[3] = 0x30; // 0
        */
        pchMessageStart[0] = 0xfb;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xb6;
        pchMessageStart[3] = 0xdb;

        nDefaultPort = 11015;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1395006231, 2085584531, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x6d39f28ad01a7edd3e2374b355cf8c7f8dbc1c5e4596ad3642fa6d10c2599217"));
        assert(genesis.hashMerkleRoot == uint256S("0xdf5d4cddc97c45da161f81032af08c18a6b7caecf0fb13d416108d61872cb618"));

        vSeeds.push_back(CDNSSeedData("seed1", "dnsseed1.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed2", "dnsseed2.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed3", "dnsseed3.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed4", "dnsseed4.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed5", "dnsseed5.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed6", "dnsseed6.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed7", "dnsseed7.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed8", "dnsseed8.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed9", "dnsseed9.egulden.org"));
        vSeeds.push_back(CDNSSeedData("seed10", "dnsseed10.egulden.org"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,48);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,176);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        // e-Gulden: Mainnet v2 enforced as of block 0
        nEnforceV2AfterHeight = 0;

        nKGWStartHeight = 45000;
        nOeruShieldMinCertifiedAddresses = 5;
        nOeruShieldMaxMasterHeightDifference = 720;
        nOeruShieldMaxBlocksSinceLastCertified = 6;
        nOeruShieldFirstMasterTXHeight = 941905;
        oeruShieldMasterKeys = {
            ParseHex("d12cdf264835fb8421734e2f3fe3da623257c993aa094ec2fd0b9645cae1930b"),
            ParseHex("b752e70e9b8343719491edfb524db6599e21d98269c1e720509636a6bb5db7ba"),
            ParseHex("f6b2c579d2bc9c86603d0689546ca989c543049d5bdd8486c9b72eee4ccca5b1")
        };

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
                ( 0,      uint256S("0x6d39f28ad01a7edd3e2374b355cf8c7f8dbc1c5e4596ad3642fa6d10c2599217"))
                ( 25000,  uint256S("0x99ffd2f7230224586c61deb459fa1cde94bda2876aed8bb791f4e3b5d485ef59"))
                ( 40000,  uint256S("0xbec732146705189352bb0007c4a26345a3ddf0eac1e8721e2fbb121b7d7c04b6"))
                ( 50000,  uint256S("0x08ac154d59f223c3c6912a03ba44487bedabd27147e0da05a7aa339fcc84e011"))
                ( 75000,  uint256S("0xd5e057b7a45fe9eb72326b5e7d7caaf31e0fd8f016425e41d8f88c9c196417c5"))
                ( 100000, uint256S("0xde49a9254ea2f01287a8b2f9d97438a52809cfd78d3c5bcc13fa529a07989083"))
                ( 125000, uint256S("0x8f817d3fdeb7fbb338d29dc8587a127c34d98617acadfd4248e4f9dfd23fd672"))
                ( 150000, uint256S("0x14eac37f76d8be510df10526e3ff053aa472d7f2e507ea3562a74b60e7174961"))
                ( 175000, uint256S("0x9d7d7db04e9dc15aa06abb45346b9a96a9136c76b6d52465eea5c3f01f861ad7"))
                ( 200000, uint256S("0x856d56f908ecde89d24ed216978f5b9898000defc15f8dcfac83a3a6eaeea9b9"))
                ( 225000, uint256S("0x1ab99f5f8ea110de1a57e128e883b760152dba409c2c1130353edcea67643277"))
                ( 250000, uint256S("0x0086da045fd362f346d1e7dcc12fe41e7eca4dc6bcd2515bdec7b7a1383c0ec9"))
                ( 300000, uint256S("0x33c0d8ea44fefa9ade4aee0032e0784e7e58ecac562129a7c0a5904472745863"))
                ( 350000, uint256S("0x1235b66a56c1c01c51902fd9bf7849e6bdf859ea2e21befa6a04cfcf0f398acf"))
                ( 400000, uint256S("0x77e2db7ef4a5704a21ae5768405f695db17be807fef7989ce16fac07eaeb5413"))
                ( 450000, uint256S("0x8e0fbd646910cfb77a49a4103267dd85f0a4157c2beaa55620301343c28ecc65"))
                ( 500000, uint256S("0x93d5ba592f1ab4a3e2a00ca72c33be19a56d1eb10bc1d5348b81b3c2408564b1"))
                ( 550000, uint256S("0x1b8bac144d11f70ee2e04b8e1e88c864d03bc32c9eee7cf903eeee8a7c6ea57e"))
                ( 600000, uint256S("0x750fe6535b77c1581282b2873f6e16c75efaaf0a4608f568bb227ecb3829618a"))
                ( 650000, uint256S("0x7d5bf58ae0f3a4bbf963fbe5b8dcd12fc04bfee2cbb2753b34ed88d8da72d048"))
                ( 700000, uint256S("0xe4d1d1729c1e8901f0eacd0687880833f05acd460671e42038ee483315adf300"))
                ( 750000, uint256S("0xf589329ce7ceb7d7dc1fa6cbae0fb4b96fb3de528a138e2e646576f41eb11eeb"))
                ( 800000, uint256S("0x062f51edbf22ff2781ab575ff42d8a3da777da82377c3e7469a45ddb7372b5ea"))
                ( 850000, uint256S("0x39c3b8797e287e4593dcbdc1fe16345f5517987e3c9e453b5c3a572bcfc93def")),
                1494449668, // * UNIX timestamp of last checkpoint block
                5502192,    // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
                5500.0      // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";

        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 400000;
        consensus.BIP34Hash = uint256S("0x860157ab24717e9694216938a9ae08cf4d58a15ca59a861e2d8469af0e6a1d50");
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 270000; // 3.125 days
        consensus.nPowTargetSpacing = 1 * 60; // 1 minute
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1483228800; // January 1, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1483228800; // January 1, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0x80; // €
        pchMessageStart[1] = 0x83; // ƒ
        pchMessageStart[2] = 0x4c; // L
        pchMessageStart[3] = 0x31; // 1
        nDefaultPort = 5744;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1504445602, 174526, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0xba8b6aaef13887bc2a69680596a905289992c897a647a1523019572892e4870a"));
        assert(genesis.hashMerkleRoot == uint256S("0xdf5d4cddc97c45da161f81032af08c18a6b7caecf0fb13d416108d61872cb618"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.push_back(CDNSSeedData("testnetseed1", "testnetseed1.egulden.org"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nKGWStartHeight = 45000;

        nOeruShieldMinCertifiedAddresses = 2;
        nOeruShieldMaxMasterHeightDifference = 720;
        nOeruShieldMaxBlocksSinceLastCertified = 6;
        nOeruShieldFirstMasterTXHeight = 100;
        oeruShieldMasterKeys = {
            ParseHex("d0686aa786f469720d681f81e7442bd99c13b58c9545973f2a7d7e24d0cdb634")
        };

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
                ( 0, uint256S("c30cbdc40e8bfcec054d3938a4def4ef70b358fc0e2ed0e85d02b2e492149ac1")),
                0,
                0,
                0
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 270000; // 3.125 days
        consensus.nPowTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0x80; // €
        pchMessageStart[1] = 0x83; // ƒ
        pchMessageStart[2] = 0x4c; // L
        pchMessageStart[3] = 0x32; // 2
        nDefaultPort = 19444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1439194624, 1, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0xc30cbdc40e8bfcec054d3938a4def4ef70b358fc0e2ed0e85d02b2e492149ac1"));
        assert(genesis.hashMerkleRoot == uint256S("0xdf5d4cddc97c45da161f81032af08c18a6b7caecf0fb13d416108d61872cb618"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nKGWStartHeight = 1000;
        nOeruShieldMinCertifiedAddresses = 1;
        nOeruShieldMaxMasterHeightDifference = 720;
        nOeruShieldMaxBlocksSinceLastCertified = 6;
        nOeruShieldFirstMasterTXHeight = 100;
        oeruShieldMasterKeys = {
            ParseHex("ffa1d249244e513c5a5ad0c27f70322008bc432b3e2e6aa4fd71c11db40ec972")
        };

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
                ( 0, uint256S("c30cbdc40e8bfcec054d3938a4def4ef70b358fc0e2ed0e85d02b2e492149ac1")),
                0,
                0,
                0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

