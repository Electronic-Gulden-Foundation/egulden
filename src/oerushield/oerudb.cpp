// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerudb.h"

#include "base58.h"
#include "crypto/sha256.h"

#include <iostream>
#include <fstream>
#include <string>

COeruDB* poeruDBMain = NULL;

void COeruDB::InitOeruDB(std::string oeruPath, bool reindex)
{
    COeruDB* oeruDBInstance = new COeruDB(oeruPath);
    assert(oeruDBInstance->ReadFile());

    poeruDBMain = oeruDBInstance;

    if (reindex) {
        poeruDBMain->ClearCertifiedAddresses();
        poeruDBMain->WriteFile();
    }
}

COeruDB::COeruDB(std::string oeruDBFileName)
{
    strOeruDBFileName = oeruDBFileName;
}

bool COeruDB::ShouldReindex(int ChainHeight)
{
    int OeruStartHeight = Params().OeruShieldFirstMasterTXHeight();
    bool OeruIsEmpty = (NumCertifiedAddresses() == 0);
    bool IsAfterStartHeight = ChainHeight > OeruStartHeight;

    return IsAfterStartHeight && OeruIsEmpty;
}

void COeruDB::AddCertifiedAddress(CBitcoinAddress addr)
{
    vOeruCertifiedAddresses.insert(addr);
}

void COeruDB::ClearCertifiedAddresses()
{
    vOeruCertifiedAddresses.clear();
}

void COeruDB::GetCertifiedAddresses(std::vector<CBitcoinAddress> &vAddresses) const
{
    vAddresses.resize(vOeruCertifiedAddresses.size());
    vAddresses.insert(
        vAddresses.begin(),
        vOeruCertifiedAddresses.begin(),
        vOeruCertifiedAddresses.end()
    );
}

bool COeruDB::IsAddressCertified(CBitcoinAddress addr) const
{
    for (auto it = vOeruCertifiedAddresses.begin(); it != vOeruCertifiedAddresses.end(); ++it)
    {
        if (*it == addr)
        {
            return true;
        }
    }

    return false;
}

int COeruDB::NumCertifiedAddresses() const
{
    return vOeruCertifiedAddresses.size();
}

void COeruDB::RemoveCertifiedAddress(CBitcoinAddress addr)
{
    for (auto it = vOeruCertifiedAddresses.begin(); it != vOeruCertifiedAddresses.end(); )
    {
        if (*it == addr)
        {
            it = vOeruCertifiedAddresses.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool COeruDB::ReadFile()
{
    std::ifstream dbfile(strOeruDBFileName);

    std::string strAddress;
    while (dbfile >> strAddress)
    {
        CBitcoinAddress addr(strAddress);

        if (addr.IsValid())
        {
            AddCertifiedAddress(addr);
        }
    }

    dbfile.close();

    return true;
}

bool COeruDB::WriteFile()
{
    std::ofstream dbfile(strOeruDBFileName, std::ios::out | std::ios::trunc);

    for (auto it = vOeruCertifiedAddresses.begin(); it != vOeruCertifiedAddresses.end(); ++it)
    {
        dbfile << it->ToString() << std::endl;
    }

    dbfile.close();

    return true;
}
