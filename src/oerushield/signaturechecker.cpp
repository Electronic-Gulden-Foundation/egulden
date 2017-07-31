// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/signaturechecker.h"

#include "base58.h"
#include "main.h"
#include "util.h"

CSignatureChecker::CSignatureChecker()
{
}

bool CSignatureChecker::VerifySignature(const std::string strMessage, const std::vector<unsigned char> vchSig, const CKeyID keyID) const
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig)) {
        std::cout << "Unable to RecoverCompact" << std::endl;
        return false;
    }

    return pubkey.GetID() == keyID;
}

bool CSignatureChecker::VerifySignature(const std::string strMessage, const std::vector<unsigned char> vchSig, const CBitcoinAddress address) const
{
    CKeyID keyID;
    if (!address.GetKeyID(keyID)) {
        std::cout << "Unable to get KeyID from " << address.ToString() << std::endl;
        LogPrint("OeruShield", "%s: Unable to get KeyID\n", __FUNCTION__);
        return false;
    }

    return VerifySignature(strMessage, vchSig, keyID);
}
