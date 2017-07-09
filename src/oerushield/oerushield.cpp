// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerushield.h"

#include "base58.h"
#include "chainparams.h"
#include "oerushield/oerudb.h"
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

bool COeruShield::FindOeruVOut(const CTransaction& coinbaseTx, CTxOut& oeruVOut) const
{
    if (!coinbaseTx.IsCoinBase())
        return false;

    for (unsigned int i = 0; i < coinbaseTx.vout.size(); i++)
    {
        oeruVOut = coinbaseTx.vout[i];

        if (HasOeruBytes(oeruVOut))
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

bool COeruShield::HasOeruBytes(const CTxOut& vout) const
{
    std::vector<unsigned char> data;
    CScript::const_iterator pc = vout.scriptPubKey.begin();
    while (pc < vout.scriptPubKey.end())
    {
        opcodetype opcode;
        if (!vout.scriptPubKey.GetOp(pc, opcode, data))
            break;

        if (opcode != OP_RETURN)
            continue;
    }

    // Check for OERU BYTES
    if (data[0] == COeruShield::OERU_BYTES[0] &&
        data[1] == COeruShield::OERU_BYTES[1] &&
        data[2] == COeruShield::OERU_BYTES[2] &&
        data[3] == COeruShield::OERU_BYTES[3]) {
      return true;
    }

    return false;
}

bool COeruShield::IsActive() const
{
    int minAddresses = Params().OeruShieldMinCertifiedAddresses();
    return oeruDB->NumCertifiedAddresses() >= minAddresses;
}

bool COeruShield::IsBlockIdentified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const
{
    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(block, coinbaseTx))
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    CTxOut oeruVOut;
    if ( ! FindOeruVOut(coinbaseTx, oeruVOut)) {
       LogPrint("OeruShield", "%s: No valid oeru vout found\n", __FUNCTION__);
       return false;
    }

    CKeyID keyID;
    if (!coinbaseAddress.GetKeyID(keyID))
        return false;

    std::vector<unsigned char> vchSig;
    CScript::const_iterator pc = oeruVOut.scriptPubKey.begin();
    while (pc < oeruVOut.scriptPubKey.end())
    {
        opcodetype opcode;
        if (!oeruVOut.scriptPubKey.GetOp(pc, opcode, vchSig))
            break;

        if (opcode != OP_RETURN)
            continue;
    }

    std::string strMessage = std::to_string(nHeight);

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    if (pubkey.GetID() == keyID) {
        LogPrint("OeruShield", "%s: Valid OERU signature\n", __FUNCTION__);
        return true;
    } else {
        LogPrint("OeruShield", "%s: No valid OERU signature\n", __FUNCTION__);
        return false;
    }
}

bool COeruShield::IsBlockCertified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const
{
    if ( ! IsBlockIdentified(block, strMessageMagic, nHeight))
        return false;

    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(block, coinbaseTx) || coinbaseTx.vout.size() < 2)
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    return oeruDB->IsAddressCertified(coinbaseAddress);
}
