// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_OERUSHIELD_H
#define BITCOIN_OERUSHIELD_OERUSHIELD_H

#include "base58.h"

#include <string>

class CBlock;
class COeruDB;

class COeruShield
{
public:
    COeruShield(COeruDB *oeruDB);

    bool IsActive() const;

    bool IsBlockIdentified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const;
    bool IsBlockCertified(const CBlock& block, const std::string strMessageMagic, const int nHeight) const;

private:
    bool GetCoinbaseAddress(const CTransaction& coinbaseTx, CBitcoinAddress& coinbaseAddress) const;
    bool GetCoinbaseTx(const CBlock& block, CTransaction& coinbaseTx) const;

    COeruDB* oeruDB = nullptr;
};

#endif // BITCOIN_OERUSHIELD_OERUSHIELD_H
