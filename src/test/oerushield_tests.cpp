// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
#include "test/test_bitcoin.h"

#include "base58.h"
#include "oerushield/oerudb.h"
#include "oerushield/oerushield.h"
#include "oerushield/oerutx.h"
#include "oerushield/signaturechecker.h"

#include <boost/test/unit_test.hpp>
#include <string>

BOOST_FIXTURE_TEST_SUITE(oerushield_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE (oerudb_certified_addresses)
{
    COeruDB oeruDB("not-used-filename.db");

    std::string addresses[] = {
        "LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5",
        "LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG",
        "LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi",
        "LVcGHJcTv1ctR6GLRXxR4SQSsycdmQ6pwZ",
        "LPD8ZwGjE4WmQ1EEnjZHrvofSyvGtbEWsH",
        "LPGeGFBPCVLHdGVD1i1oikzD92XZoTEVyh"
    };

    // Add some certified addresses
    oeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[0]));
    oeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[1]));
    oeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[2]));

    // Add one double to check uniqueness enforced
    oeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[0]));

    BOOST_CHECK(oeruDB.NumCertifiedAddresses() == 3);

    // Check certified == true
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[0])) == true);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[1])) == true);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[2])) == true);

    // Check certified == false
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[3])) == false);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[4])) == false);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[5])) == false);

    // Remove addresses and check certified == false
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress(addresses[0]));
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress(addresses[1]));
    oeruDB.RemoveCertifiedAddress(CBitcoinAddress(addresses[2]));

    BOOST_CHECK(oeruDB.NumCertifiedAddresses() == 0);

    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[0])) == false);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[1])) == false);
    BOOST_CHECK(oeruDB.IsAddressCertified(CBitcoinAddress(addresses[2])) == false);
}

BOOST_AUTO_TEST_CASE (oerudb_read_write_dbfile)
{
    boost::filesystem::path tmpfile = boost::filesystem::temp_directory_path() / "oeru.db";

    COeruDB writeOeruDB(tmpfile.string().c_str());

    std::string addresses[] = {
        "LdwLvykqj2nUH3MWcut6mtjHxVxVFC7st5",
        "LWZR9ybwmT8vSXP6tmrBX4b6nE9o94AjQG",
        "LWkdEB9SHUfuBiTvZofK2LqYE4RTTtUcqi"
    };

    writeOeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[0]));
    writeOeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[1]));
    writeOeruDB.AddCertifiedAddress(CBitcoinAddress(addresses[2]));

    BOOST_CHECK(writeOeruDB.NumCertifiedAddresses() == 3);

    writeOeruDB.WriteFile();

    COeruDB readOeruDB(tmpfile.string().c_str());
    readOeruDB.ReadFile();

    BOOST_CHECK(readOeruDB.NumCertifiedAddresses() == 3);

    // Check certified == true
    BOOST_CHECK(readOeruDB.IsAddressCertified(CBitcoinAddress(addresses[0])) == true);
    BOOST_CHECK(readOeruDB.IsAddressCertified(CBitcoinAddress(addresses[1])) == true);
    BOOST_CHECK(readOeruDB.IsAddressCertified(CBitcoinAddress(addresses[2])) == true);
}

BOOST_AUTO_TEST_CASE (oerudb_is_master_key)
{
    COeruShield oeruShield(nullptr);

    BOOST_CHECK(oeruShield.IsMasterKey(ParseHex("1e58eb7273d4ce30e9a961600aaa49871beec551aba5b6f5a5712d6ccd1a8e3a")) == true);
    BOOST_CHECK(oeruShield.IsMasterKey(ParseHex("b752e70e9b8343719491edfb524db6599e21d98269c1e720509636a6bb5db7ba")) == true);

    BOOST_CHECK(oeruShield.IsMasterKey(CBitcoinAddress("LLUAaniHSW6eH1QQUrJ7ZAEHurkhx857f3")) == false);
    BOOST_CHECK(oeruShield.IsMasterKey(CBitcoinAddress("LQHK6ejxSbjnu4XKa1XjprjmPhrtPdiJaG")) == false);
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_invalid)
{
    std::vector<unsigned char> data = ParseHex("203fb6ba2a53b41cba97a2ddbaddb52a5b942dd450b6873d8ca633a07eee74820004ce88a1a2eca319a8f02189e82c3ce186d5cc015e60e12e266017ac02c623a8");
    COeruMasterData masterData(&data);

    BOOST_CHECK(masterData.IsValid() == false);
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_valid)
{
    CBitcoinAddress signingAddress("LaZ27rggR2KnmvVGxa3kzkoqxgDYidti2k");
    CBitcoinAddress nonSigningAddress("LgEHSpv22knkaSR1ZbPSaxqtXujReQykK9");

    std::vector<unsigned char> data = ParseHex("4f45525501000f4240208289659af9426a9e1a6540b9d6da97e942ff308c0a537cce764eac9dec8012565be84a321a7dd9eca5eaf4299ef67311e0e7a964a35e9bcdb8f3d12f952d8422");
    COeruMasterData masterData(&data);

    BOOST_CHECK(masterData.HasOeruBytes() == true);
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

    std::string rawMessage;
    BOOST_CHECK(masterData.GetRawMessage(rawMessage) == true);
    BOOST_CHECK(rawMessage == "01000f4240");

    BOOST_CHECK(masterData.HasValidSignature(signingAddress) == true);
    BOOST_CHECK(masterData.HasValidSignature(nonSigningAddress) == false);
}

BOOST_AUTO_TEST_CASE (oerusignature_checker)
{
    CBitcoinAddress addr1("LaZ27rggR2KnmvVGxa3kzkoqxgDYidti2k");
    CBitcoinAddress addr2("LgEHSpv22knkaSR1ZbPSaxqtXujReQykK9");

    std::string msg1 = "test";
    std::string msg2 = "some-string";

    std::vector<unsigned char> sig1 = ParseHex("1fd18444a624831c6799647edfc061d6462f47540e7f3a96332f57aa1958946d070820bc47f30472487527dcc165591b278ab8f676bede169c2572b1bda39d8a0e");
    std::vector<unsigned char> sig2 = ParseHex("20e1dbd00c8497433a746c12733c3d3f21e4a993057c50438254b708ce3f5d120a6ee282c1571b5fb4a101501f5291ceb17094aff72dd0fe38947594d2d2e7e766");

    CSignatureChecker sigChecker;

    BOOST_CHECK(sigChecker.VerifySignature(msg1, sig1, addr1) == true);
    BOOST_CHECK(sigChecker.VerifySignature(msg2, sig2, addr2) == true);

    BOOST_CHECK(sigChecker.VerifySignature(msg1, sig1, addr2) == false);
    BOOST_CHECK(sigChecker.VerifySignature(msg2, sig2, addr1) == false);
}

BOOST_AUTO_TEST_SUITE_END()
