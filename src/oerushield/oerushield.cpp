// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerushield.h"

#include "base58.h"
#include "chainparams.h"
#include "oerushield/oerudb.h"
#include "oerushield/oerutx.h"
#include "oerushield/signaturechecker.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "util.h"

#include <string>
#include <vector>

const std::vector<unsigned char> COeruShield::OERU_BYTES = { 0x4f, 0x45, 0x52, 0x55 }; // "OERU"

COeruShield::COeruShield(COeruDB *oeruDB)
{
    this->oeruDB = oeruDB;
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

bool COeruShield::GetCoinbaseAddress(const CTransaction& coinbaseTx, CBitcoinAddress& coinbaseAddress) const
{
    if (coinbaseTx.vout.size() < 1)
        return false;

    CTxOut coinbase = coinbaseTx.vout[0];

    CTxDestination coinbaseDest;
    if (!ExtractDestination(coinbase.scriptPubKey, coinbaseDest))
        return false;

    coinbaseAddress = CBitcoinAddress(coinbaseDest);
    if (!coinbaseAddress.IsValid())
        return false;

    return true;
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

    std::vector<unsigned char> vchSig;
    if (!oeruTxOut.GetOpReturnData(vchSig)) {
       LogPrint("OeruShield", "%s: No OP_RETURN data found\n", __FUNCTION__);
       return false;
    }

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
