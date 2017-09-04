// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_SIGNATURECHECKER_H
#define BITCOIN_OERUSHIELD_SIGNATURECHECKER_H

#include <string>
#include <vector>

class CBitcoinAddress;
class CKeyID;

class CSignatureChecker
{
public:
    CSignatureChecker();

    bool VerifySignature(const std::string strMessage, const std::vector<unsigned char> vchSig, const CBitcoinAddress address) const;
};

#endif // BITCOIN_OERUSHIELD_SIGNATURECHECKER_H
