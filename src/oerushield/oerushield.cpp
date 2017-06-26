// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerushield.h"

#include "base58.h"
#include "chainparams.h"
#include "oerushield/oerudb.h"
#include "primitives/block.h"
#include "primitives/transaction.h"

#include <string>

COeruShield::COeruShield(COeruDB *oeruDB)
{
    this->oeruDB = oeruDB;
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

bool COeruShield::IsBlockIdentified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const
{
    CTransaction coinbaseTx;
    if ( ! GetCoinbaseTx(block, coinbaseTx) || coinbaseTx.vout.size() < 2)
        return false;

    CBitcoinAddress coinbaseAddress;
    if ( ! GetCoinbaseAddress(coinbaseTx, coinbaseAddress))
        return false;

    CTxOut oeruTXOut = coinbaseTx.vout[1];

    CKeyID keyID;
    if (!coinbaseAddress.GetKeyID(keyID))
        return false;

    std::vector<unsigned char> vchSig;
    CScript::const_iterator pc = oeruTXOut.scriptPubKey.begin();
    while (pc < oeruTXOut.scriptPubKey.end())
    {
        opcodetype opcode;
        if (!oeruTXOut.scriptPubKey.GetOp(pc, opcode, vchSig))
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

    return (pubkey.GetID() == keyID);
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
