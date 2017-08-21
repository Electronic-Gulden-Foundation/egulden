// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerushield.h"

#include "base58.h"
#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "oerushield/oerudb.h"
#include "oerushield/oerutx.h"
#include "oerushield/signaturechecker.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "util.h"
#include "utilstrencodings.h"

#include <set>
#include <string>
#include <vector>

const std::vector<unsigned char> COeruShield::OERU_BYTES = { 0x4f, 0x45, 0x52, 0x55 }; // "OERU"
const uint64_t COeruShield::MAX_HEIGHT_DIFFERENCE = 720;

const std::set<std::vector<unsigned char>> COeruShield::MASTER_KEYS =
{
    ParseHex("1e58eb7273d4ce30e9a961600aaa49871beec551aba5b6f5a5712d6ccd1a8e3a")
    , ParseHex("b752e70e9b8343719491edfb524db6599e21d98269c1e720509636a6bb5db7ba")
    , ParseHex("f6b2c579d2bc9c86603d0689546ca989c543049d5bdd8486c9b72eee4ccca5b1")
};

COeruShield::COeruShield(COeruDB *oeruDB)
{
    this->oeruDB = oeruDB;
}

bool COeruShield::AcceptBlock(const CBlock& block, const CBlockIndex *pindexPrev) const
{
    // Genesis block must always be accepted
    if (pindexPrev == NULL)
        return true;

    int blocksSinceLastCertified = GetBlocksSinceLastCertified(block, pindexPrev);

    int nHeight = pindexPrev->nHeight + 1;
    LogPrint("OeruShield", "OERU @ Block %d:\n\t- Active: %d\n\t- Identified: %d\n\t- Certified: %d\n\t- Last certified: %d\n",
            nHeight,
            IsActive(),
            IsBlockIdentified(block, nHeight),
            IsBlockCertified(block, nHeight),
            GetBlocksSinceLastCertified(block, pindexPrev));

    return (blocksSinceLastCertified != -1 &&
            blocksSinceLastCertified <= MAX_BLOCKS_SINCE_LAST_CERTIFIED);
}

bool COeruShield::CheckMasterTx(CTransaction tx, int nHeight) const
{
    if (tx.IsCoinBase())
        return false;

    if (tx.vout.size() != 3)
        return false;

    CTxOut minerOut = tx.vout[0];
    CTxOut masterOut = tx.vout[1];
    CTxOut signatureOut = tx.vout[2];

    CBitcoinAddress master;
    if (!GetDestinationAddress(masterOut, master))
        return false;

    if (!IsMasterKey(master))
        return false;

    CBitcoinAddress miner;
    if (!GetDestinationAddress(minerOut, miner))
        return false;

    COeruTxOut masterOeruOut(&signatureOut);
    COeruMasterData masterData = masterOeruOut.GetOeruMasterData();

    if (!masterData.IsValid())
        return false;

    uint64_t masterHeight;
    if (!masterData.GetHeight(masterHeight))
        return false;

    if (masterHeight > nHeight + COeruShield::MAX_HEIGHT_DIFFERENCE)
        return false;

    if (!masterData.HasValidSignature(master, miner))
        return false;

    bool enable;
    if (!masterData.GetEnable(enable))
        return false;

    if (enable)
        oeruDB->AddCertifiedAddress(miner);
    else
        oeruDB->RemoveCertifiedAddress(miner);

    oeruDB->WriteFile();

    return true;
}

bool COeruShield::FindOeruVOut(const CTransaction& coinbaseTx, COeruTxOut& oeruTxOut) const
{
    if (!coinbaseTx.IsCoinBase())
        return false;

    for (auto &vout : coinbaseTx.vout)
    {
        oeruTxOut = COeruTxOut(&vout);

        if (oeruTxOut.HasOeruBytes())
        {
            return true;
        }
    }
    return false;
}

int COeruShield::GetBlocksSinceLastCertified(const CBlock& block, const CBlockIndex *pindexPrev) const
{
    // Pretend that the genesis block was certified
    if (pindexPrev == NULL)
        return 0;

    if (IsBlockCertified(block, pindexPrev->nHeight + 1)) return 0;

    const CBlockIndex *pcurIndex = pindexPrev;

    for (int i = 1; i <= MAX_BLOCKS_SINCE_LAST_CERTIFIED + 1; i++)
    {
        // We arrived at the genesis block, which is 'certified'
        if (pcurIndex == NULL)
            return i;

        CBlock prevblock;
        ReadBlockFromDisk(prevblock, pcurIndex, Params().GetConsensus());

        if (IsBlockCertified(prevblock, pcurIndex->nHeight))
            return i;

        pcurIndex = pcurIndex->pprev;
    }

    return -1;
}

bool COeruShield::GetCoinbaseAddress(const CTransaction& coinbaseTx, CBitcoinAddress& coinbaseAddress) const
{
    if (coinbaseTx.vout.size() < 1)
        return false;

    return GetDestinationAddress(coinbaseTx.vout[0], coinbaseAddress);
}

bool COeruShield::GetCoinbaseTx(const CBlock& block, CTransaction& coinbaseTx) const
{
    if (block.vtx.size() < 1)
        return false;

    coinbaseTx = block.vtx[0];

    if (!coinbaseTx.IsCoinBase())
        return false;

    return true;
}

bool COeruShield::GetDestinationAddress(const CTxOut txOut, CBitcoinAddress &destination) const
{
    CTxDestination txDestination;
    if (!ExtractDestination(txOut.scriptPubKey, txDestination))
        return false;

    destination = CBitcoinAddress(txDestination);
    if (!destination.IsValid())
        return false;

    return true;
}

bool COeruShield::IsActive() const
{
    int minAddresses = Params().OeruShieldMinCertifiedAddresses();
    return oeruDB->NumCertifiedAddresses() >= minAddresses;
}

bool COeruShield::IsBlockIdentified(const CBlock& block, const int nHeight) const
{
    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(block, coinbaseTx))
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    COeruTxOut oeruTxOut;
    if ( ! FindOeruVOut(coinbaseTx, oeruTxOut)) {
       LogPrint("OeruShield", "%s: No valid oeru vout found\n", __FUNCTION__);
       return false;
    }

    std::vector<unsigned char> vchData;
    if (!oeruTxOut.GetOpReturnData(vchData)) {
       LogPrint("OeruShield", "%s: No OP_RETURN data found\n", __FUNCTION__);
       return false;
    }

    std::vector<unsigned char> vchSig(vchData.begin() + COeruShield::OERU_BYTES.size(), vchData.end());
    std::string strMessage = std::to_string(nHeight);

    CSignatureChecker signatureChecker;
    if (signatureChecker.VerifySignature(strMessage, vchSig, coinbaseAddress)) {
        LogPrint("OeruShield", "%s: Valid OERU signature\n", __FUNCTION__);
        return true;
    } else {
        LogPrint("OeruShield", "%s: No valid OERU signature\n", __FUNCTION__);
        return false;
    }
}

bool COeruShield::IsBlockCertified(const CBlock& block, const int nHeight) const
{
    if ( ! IsBlockIdentified(block, nHeight))
        return false;

    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(block, coinbaseTx) || coinbaseTx.vout.size() < 2)
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    return oeruDB->IsAddressCertified(coinbaseAddress);
}

bool COeruShield::IsMasterKey(std::vector<unsigned char> addrHash) const
{
    return MASTER_KEYS.count(addrHash) == 1;
}

bool COeruShield::IsMasterKey(CBitcoinAddress addr) const
{
    std::string strAddr = addr.ToString();
    std::vector<unsigned char> hash;
    hash.resize(CSHA256::OUTPUT_SIZE);

    CSHA256().Write((unsigned char*) &strAddr[0], strAddr.size())
        .Finalize(&hash[0]);

    return IsMasterKey(hash);
}
