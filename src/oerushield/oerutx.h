// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_OERUTX_H
#define BITCOIN_OERUSHIELD_OERUTX_H

#include <stdint.h>
#include <string>
#include <vector>

class CBitcoinAddress;
class CTxOut;

class COeruMasterData
{
public:
    COeruMasterData();
    COeruMasterData(const std::vector<unsigned char> *data);

    bool GetEnable(bool &out) const;
    bool GetHeight(uint64_t &out) const;
    bool GetRawMessage(std::string &out) const;
    bool GetSignature(std::vector<unsigned char> &vchSig) const;

    bool HasOeruBytes() const;
    bool HasValidSignature(CBitcoinAddress address) const;

    bool IsValid() const;
private:
    const std::vector<unsigned char> *data = nullptr;

    // Lengths
    unsigned int nOeruBytesLength = 4;
    unsigned int nEnableLength = 1;
    unsigned int nHeightLength = 4;
    unsigned int nSignatureLength = 65;
    unsigned int nTotalLength = nOeruBytesLength + nEnableLength + nHeightLength + nSignatureLength;

    // Locations
    unsigned int nOeruBytesStart = 0;
    unsigned int nOeruBytesEnd = nOeruBytesStart + nOeruBytesLength;
    unsigned int nEnableStart = nOeruBytesEnd;
    unsigned int nEnableEnd = nEnableStart + nEnableLength - 1;
    unsigned int nHeightStart = 1 + nEnableEnd;
    unsigned int nHeightEnd = nHeightStart + nHeightLength - 1;
    unsigned int nSignatureStart = 1 + nHeightEnd;
    unsigned int nSignatureEnd = nSignatureStart + nSignatureLength;
};

class COeruTxOut
{
public:
    COeruTxOut();
    COeruTxOut(const CTxOut *vout);

    bool HasOeruBytes() const;
    bool GetOpReturnData(std::vector<unsigned char> &data) const;
    bool GetOeruMasterData(COeruMasterData &masterData) const;
private:
    const CTxOut* vout = nullptr;
};

#endif // BITCOIN_OERUSHIELD_OERUTX_H
