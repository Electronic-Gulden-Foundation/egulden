// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerutx.h"

#include "oerushield/oerushield.h"
#include "oerushield/signaturechecker.h"
#include "primitives/transaction.h"
#include "util.h"
#include "utilstrencodings.h"

#include <vector>
#include <boost/format.hpp>

COeruTxOut::COeruTxOut()
{
    this->vout = nullptr;
}

COeruTxOut::COeruTxOut(const CTxOut *vout)
{
    this->vout = vout;
}

bool COeruTxOut::GetOpReturnData(std::vector<unsigned char> &data) const
{
    if (vout == nullptr) {
        LogPrint("OeruShield", "%s: vout is nullptr\n", __FUNCTION__);
        return false;
    }

    CScript script = vout->scriptPubKey;
    CScript::const_iterator pc = script.begin();
    while (pc < script.end())
    {
        opcodetype opcode;
        if (!script.GetOp(pc, opcode, data))
            break;

        if (opcode != OP_RETURN)
            continue;
    }

    if (data.size() > 0) {
        LogPrint("OeruShield", "%s: Found data (size: %d)\n",
                __FUNCTION__,  data.size());
        return true;
    }

    return false;
}

bool COeruTxOut::HasOeruBytes() const
{
    std::vector<unsigned char> data;
    if (!GetOpReturnData(data)) {
        LogPrint("OeruShield", "%s: No OP_RETURN data\n", __FUNCTION__);
        return false;
    }

    if (data.size() < 4) {
        LogPrint("OeruShield", "%s: OP_RETURN data too small (size: %d)\n",
                __FUNCTION__, data.size());
        return false;
    }

    // Check for OERU BYTES
    if (data[0] == COeruShield::OERU_BYTES[0] &&
        data[1] == COeruShield::OERU_BYTES[1] &&
        data[2] == COeruShield::OERU_BYTES[2] &&
        data[3] == COeruShield::OERU_BYTES[3]) {
        LogPrint("OeruShield", "%s: OERU bytes found\n", __FUNCTION__);
      return true;
    }

    LogPrint("OeruShield", "%s: No OERU bytes found\n", __FUNCTION__);
    return false;
}

COeruMasterData COeruTxOut::GetOeruMasterData() const
{
    std::vector<unsigned char> data;
    if (!GetOpReturnData(data)) {
        LogPrint("OeruShield", "%s: No OP_RETURN data\n", __FUNCTION__);
        return COeruMasterData();
    }

    return COeruMasterData(data);
}

COeruMasterData::COeruMasterData()
{
}

COeruMasterData::COeruMasterData(std::vector<unsigned char> pvchData)
{
    data = pvchData;
}

bool COeruMasterData::GetEnable(bool &out) const
{
    if (!IsValid()) return false;

    out = *(data.begin() + nEnableStart);
    return true;
}

bool COeruMasterData::GetHeight(uint64_t &out) const
{
    if (!IsValid()) return false;

    out  = 0;
    out |= (*(data.begin() + nHeightStart + 0) << 24);
    out |= (*(data.begin() + nHeightStart + 1) << 16);
    out |= (*(data.begin() + nHeightStart + 2) << 8 );
    out |= (*(data.begin() + nHeightStart + 3)      );

    return true;
}

bool COeruMasterData::GetRawMessage(std::string &out) const
{
    if (!IsValid()) return false;

    bool enable;
    if (!GetEnable(enable))
        return false;

    uint64_t height;
    if (!GetHeight(height))
        return false;

    std::stringstream ss;
    ss << boost::format("%02x%08x") % enable % height;

    out = ss.str();

    return true;
}

bool COeruMasterData::GetSignature(std::vector<unsigned char> &vchSig) const
{
    if (!IsValid()) return false;

    vchSig.insert(
        vchSig.begin(),
        data.begin() + nSignatureStart,
        data.begin() + nSignatureEnd
    );

    return true;
}

bool COeruMasterData::HasOeruBytes() const
{
    if (data.size() < nOeruBytesLength)
        return false;

    std::vector<unsigned char> oeruBytes(
        data.begin() + nOeruBytesStart,
        data.begin() + nOeruBytesLength
    );

    if (oeruBytes != COeruShield::OERU_BYTES)
    {
        LogPrint("OeruShield", "%s: No oeru bytes in oeru master data\n", __FUNCTION__);
        return false;
    }

    return true;
}

bool COeruMasterData::HasValidSignature(CBitcoinAddress master, CBitcoinAddress miner) const
{
    if (!IsValid()) return false;

    std::vector<unsigned char> vchSig;
    if (!GetSignature(vchSig))
        return false;

    std::string rawMessage;
    if (!GetRawMessage(rawMessage))
        return false;

    std::string signedMessage = rawMessage + miner.ToString();

    CSignatureChecker sigChecker;
    return sigChecker.VerifySignature(signedMessage, vchSig, master);
}

bool COeruMasterData::IsValid() const
{
    if (data.size() != nTotalLength) {
        LogPrint("OeruShield", "%s: master data is not of expected length %d (was %d)\n",
                __FUNCTION__, nTotalLength, data.size());
        return false;
    }

    return HasOeruBytes();
}
