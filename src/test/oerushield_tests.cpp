// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerudb.h"
#include "oerushield/oerushield.h"
#include "oerushield/oerutx.h"
#include "test/test_bitcoin.h"

#include "base58.h"

#include <boost/test/unit_test.hpp>
#include <string>

BOOST_FIXTURE_TEST_SUITE(oerushield_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE (oerushield_certified_addresses)
{
    COeruDB oeruDB("not-used-filename.db");

    // Add some certified addresses
    oeruDB.AddCertifiedAddress(CBitcoinAddress("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5"));
    oeruDB.AddCertifiedAddress(CBitcoinAddress("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG"));
    oeruDB.AddCertifiedAddress(CBitcoinAddress("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi"));

    // Add one double to check uniqueness enforced
    oeruDB.AddCertifiedAddress(CBitcoinAddress("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi"));

    BOOST_CHECK(oeruDB.NumCertifiedAddresses() == 3);

    // Check certified == true
    {
        CBitcoinAddress toCheck("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == true);
    }
    {
        CBitcoinAddress toCheck("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == true);
    }
    {
        CBitcoinAddress toCheck("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == true);
    }

    // Check certified == false
    {
        CBitcoinAddress toCheck("LVcGHJcTv1ctR6GLRXxR4SQSsycdmQ6pwZ");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }
    {
        CBitcoinAddress toCheck("LPD8ZwGjE4WmQ1EEnjZHrvofSyvGtbEWsH");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }
    {
        CBitcoinAddress toCheck("LPGeGFBPCVLHdGVD1i1oikzD92XZoTEVyh");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }

    // Remove addresses and check certified == false
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5"));
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG"));
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi"));

    BOOST_CHECK(oeruDB.NumCertifiedAddresses() == 0);

    {
        CBitcoinAddress toCheck("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }
    {
        CBitcoinAddress toCheck("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }
    {
        CBitcoinAddress toCheck("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi");
        BOOST_CHECK(oeruDB.IsAddressCertified(toCheck) == false);
    }
}

BOOST_AUTO_TEST_CASE (oerushield_read_write_dbfile)
{
    boost::filesystem::path tmpfile = boost::filesystem::temp_directory_path() / "oeru.db";

    COeruDB writeOeruDB(tmpfile.string().c_str());

    writeOeruDB.AddCertifiedAddress(CBitcoinAddress("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5"));
    writeOeruDB.AddCertifiedAddress(CBitcoinAddress("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG"));
    writeOeruDB.AddCertifiedAddress(CBitcoinAddress("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi"));

    BOOST_CHECK(writeOeruDB.NumCertifiedAddresses() == 3);

    writeOeruDB.WriteFile();

    COeruDB readOeruDB(tmpfile.string().c_str());
    readOeruDB.ReadFile();

    BOOST_CHECK(readOeruDB.NumCertifiedAddresses() == 3);

    // Check certified == true
    {
        CBitcoinAddress toCheck("LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5");
        BOOST_CHECK(readOeruDB.IsAddressCertified(toCheck) == true);
    }
    {
        CBitcoinAddress toCheck("LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG");
        BOOST_CHECK(readOeruDB.IsAddressCertified(toCheck) == true);
    }
    {
        CBitcoinAddress toCheck("LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi");
        BOOST_CHECK(readOeruDB.IsAddressCertified(toCheck) == true);
    }
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_invalid)
{
    std::vector<unsigned char> data = ParseHex("");
    COeruMasterData masterData(&data);

    BOOST_CHECK(masterData.IsValid() == false);
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_valid)
{
    std::vector<unsigned char> data = ParseHex("01000f4240203fb6ba2a53b41cba97a2ddbaddb52a5b942dd450b6873d8ca633a07eee74820004ce88a1a2eca319a8f02189e82c3ce186d5cc015e60e12e266017ac02c623a8");
    COeruMasterData masterData(&data);

    BOOST_CHECK(masterData.IsValid() == true);

    bool enable;
    BOOST_CHECK(masterData.GetEnable(enable) == true);
    BOOST_CHECK(enable == true);

    uint64_t height;
    BOOST_CHECK(masterData.GetHeight(height) == true);

    BOOST_CHECK(height == 1000000);

    std::vector<unsigned char> sig;
    BOOST_CHECK(masterData.GetSignature(sig) == true);
    BOOST_CHECK(sig.size() == 65);
}

BOOST_AUTO_TEST_SUITE_END()
