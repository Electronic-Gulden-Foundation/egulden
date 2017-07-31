// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_OERUSHIELD_OERUDB_H
#define BITCOIN_OERUSHIELD_OERUDB_H

#include <set>
#include <string>

class CBitcoinAddress;

class COeruDB
{
public:
    static void InitOeruDB(std::string oeruDBPath);

    COeruDB(std::string oeruDBFileName);

    void AddCertifiedAddress(CBitcoinAddress addr);
    bool IsAddressCertified(CBitcoinAddress addr) const;
    int NumCertifiedAddresses();
    void RemoveCertifiedAddress(CBitcoinAddress addr);

    // Reads the data from the file
    bool ReadFile();

    // Writes the data to the file
    bool WriteFile();

private:
    // Use a set to guarantee unique entries
    std::set<CBitcoinAddress> vOeruCertifiedAddresses;

    // Database path
    std::string strOeruDBFileName;
};

extern COeruDB* poeruDBMain;

#endif // BITCOIN_OERUSHIELD_OERUDB_H
