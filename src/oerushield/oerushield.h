// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_OERUSHIELD_H
#define BITCOIN_OERUSHIELD_OERUSHIELD_H

#include "base58.h"

#include <string>
#include <vector>

class CBlock;
class COeruDB;
class CTxOut;

class COeruShield
{
public:
    static const std::vector<unsigned char> OERU_BYTES; // "OERU"

    COeruShield(COeruDB *oeruDB);

    bool IsActive() const;

    bool IsBlockIdentified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const;
    bool IsBlockCertified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const;

private:
    bool FindOeruVOut(const CTransaction& coinbaseTx, CTxOut& oeruVOut) const;
    bool GetCoinbaseAddress(const CTransaction& coinbaseTx, CBitcoinAddress& coinbaseAddress) const;
    bool GetCoinbaseTx(const CBlock& block, CTransaction& coinbaseTx) const;
    bool HasOeruBytes(const CTxOut& vout) const;

    COeruDB* oeruDB = nullptr;
};

#endif // BITCOIN_OERUSHIELD_OERUSHIELD_H
