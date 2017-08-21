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

bool CSignatureChecker::VerifySignature(const std::string strMessage, const std::vector<unsigned char> vchSig, const CBitcoinAddress address) const
{
    if (!address.IsValid()) {
        LogPrint("OeruShield", "%s: Address is invalid\n", __FUNCTION__);
        return false;
    }

    CKeyID keyID;
    if (!address.GetKeyID(keyID)) {
        LogPrint("OeruShield", "%s: Unable to get KeyID\n", __FUNCTION__);
        return false;
    }

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig)) {
        LogPrint("OeruShield", "%s: Unable to RecoverCompact\n", __FUNCTION__);
        return false;
    }

    return pubkey.GetID() == keyID;
}
