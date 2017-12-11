// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_OERUSIGNAL_H
#define BITCOIN_OERUSHIELD_OERUSIGNAL_H

#include <ctime>
#include <string>

class COeruSignal
{
public:
    static void InitOeruSignal(std::string strUAComment);

    COeruSignal(std::string strUAComment);

    std::string CreateSignalPath(int nBlockHeight);
    bool ExecuteOeruSignal(int nBlockHeight);
private:
    std::string hostname = "uasignal.e-gulden.org";
    std::string strUAComment;
    int nNextOeruSignalExecutionHeight = 0;
    std::time_t tLastRequestTime = 0;
};

extern COeruSignal* poeruSignalMain;

#endif // BITCOIN_OERUSHIELD_OERUSIGNAL_H
