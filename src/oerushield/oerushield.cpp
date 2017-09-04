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

COeruShield::COeruShield(COeruDB *oeruDB)
{
    this->oeruDB = oeruDB;
}

bool COeruShield::AcceptBlock(const CBlock *pblock, const CBlockIndex *pindex) const
{
    // Genesis block must always be accepted
    if (pindex->nHeight == 0)
        return true;

    int blocksSinceLastCertified = GetBlocksSinceLastCertified(pblock, pindex);
    LogPrint("OeruShield", "OERU @ Block %d:\n\t- Active: %d\n\t- Identified: %d\n\t- Certified: %d\n\t- Last certified: %d\n",
            pindex->nHeight,
            IsActive(),
            IsBlockIdentified(pblock, pindex->nHeight),
            IsBlockCertified(pblock, pindex->nHeight),
            blocksSinceLastCertified);

    return (blocksSinceLastCertified >= 0 &&
            blocksSinceLastCertified <  Params().OeruShieldMaxBlocksSinceLastCertified());
}

bool COeruShield::CheckMasterTx(const CTransaction tx, const int nHeight, const bool revert) const
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

    if (!masterData.IsValid()) {
        LogPrint("OeruShield", "%s: No valid master data found\n", __FUNCTION__);
        return false;
    }

    bool enable;
    if (!masterData.GetEnable(enable)) {
        LogPrint("OeruShield", "%s: Master TX failed getting enable status\n", __FUNCTION__);
        return false;
    }

    uint64_t masterHeight;
    if (!masterData.GetHeight(masterHeight)) {
        LogPrint("OeruShield", "%s: Master TX failed getting height\n", __FUNCTION__);
        return false;
    }

    if (masterHeight > nHeight + Params().OeruShieldMaxMasterHeightDifference()) {
        LogPrint("OeruShield", "%s: Master TX too old\n", __FUNCTION__);
        return false;
    }

    if (!masterData.HasValidSignature(master, miner)) {
        LogPrint("OeruShield", "%s: Master TX has no valid signature\n", __FUNCTION__);
        return false;
    }

    // If this is a 'revert check' (ie called from DisconnectTip)
    // we need to invert the action
    if (revert)
        enable = !enable;

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

int COeruShield::GetBlocksSinceLastCertified(const CBlock *pblock, const CBlockIndex *pindex, int i) const
{
    // Pretend that the genesis block was certified
    if (pindex == NULL || pindex->pprev == NULL)
        return i;

    if (IsBlockCertified(pblock, pindex->nHeight))
        return i;

    // Don't read further back than we need to
    if (i > Params().OeruShieldMaxBlocksSinceLastCertified())
        return -1;

    CBlock prevblock;
    ReadBlockFromDisk(prevblock, pindex->pprev, Params().GetConsensus());

    return GetBlocksSinceLastCertified(&prevblock, pindex->pprev, i + 1);
}

bool COeruShield::GetCoinbaseAddress(const CTransaction& coinbaseTx, CBitcoinAddress& coinbaseAddress) const
{
    if (coinbaseTx.vout.size() < 1)
        return false;

    return GetDestinationAddress(coinbaseTx.vout[0], coinbaseAddress);
}

bool COeruShield::GetCoinbaseTx(const CBlock *pblock, CTransaction& coinbaseTx) const
{
    if (pblock->vtx.size() < 1)
        return false;

    coinbaseTx = pblock->vtx[0];

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
    if (oeruDB == nullptr)
        return false;

    int minAddresses = Params().OeruShieldMinCertifiedAddresses();
    return oeruDB->NumCertifiedAddresses() >= minAddresses;
}

bool COeruShield::IsBlockIdentified(const CBlock *pblock, const int nHeight) const
{
    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(pblock, coinbaseTx))
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

    std::vector<unsigned char> vchSig(
        vchData.begin() + COeruShield::OERU_BYTES.size(),
        vchData.end()
    );
    std::string strMessage = std::to_string(nHeight);

    CSignatureChecker signatureChecker;
    if (signatureChecker.VerifySignature(strMessage, vchSig, coinbaseAddress)) {
        LogPrint("OeruShield", "%s: Valid OERU signature on block %d\n",
                __FUNCTION__, nHeight);
        return true;
    } else {
        LogPrint("OeruShield", "%s: No valid OERU signature on block %d\n",
                __FUNCTION__, nHeight);
        return false;
    }
}

bool COeruShield::IsBlockCertified(const CBlock *pblock, const int nHeight) const
{
    if ( ! IsBlockIdentified(pblock, nHeight))
        return false;

    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(pblock, coinbaseTx) || coinbaseTx.vout.size() < 2)
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    return oeruDB->IsAddressCertified(coinbaseAddress);
}

bool COeruShield::IsMasterKey(std::vector<unsigned char> addrHash) const
{
    return Params().OeruShieldMasterKeys().count(addrHash) == 1;
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
