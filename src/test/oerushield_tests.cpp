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

std::string GetTempFilePath()
{
    boost::filesystem::path tmpfile = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    return tmpfile.string().c_str();
}

BOOST_AUTO_TEST_CASE (oerudb_certified_addresses)
{
    COeruDB oeruDB(GetTempFilePath());

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

BOOST_AUTO_TEST_CASE (oerudb_reindex)
{
    COeruDB oeruDB(GetTempFilePath());

    // OeruShieldFirstMasterTXHeight: 941905 on mainnet
    BOOST_CHECK(oeruDB.ShouldReindex(940000) == false);
    BOOST_CHECK(oeruDB.ShouldReindex(950000) == true);

    // Add one certified address, expect no reindex necessary as
    // OERUDB was presumably initialized before the first master TX
    oeruDB.AddCertifiedAddress(CBitcoinAddress("LLUAaniHSW6eH1QQUrJ7ZAEHurkhx857f3"));

    BOOST_CHECK(oeruDB.ShouldReindex(940000) == false);
    BOOST_CHECK(oeruDB.ShouldReindex(950000) == false);
}

BOOST_AUTO_TEST_CASE (oerushield_is_master_key)
{
    COeruShield oeruShield(nullptr);

    BOOST_CHECK(oeruShield.IsMasterKey(ParseHex("d12cdf264835fb8421734e2f3fe3da623257c993aa094ec2fd0b9645cae1930b")) == true);
    BOOST_CHECK(oeruShield.IsMasterKey(ParseHex("b752e70e9b8343719491edfb524db6599e21d98269c1e720509636a6bb5db7ba")) == true);

    BOOST_CHECK(oeruShield.IsMasterKey(CBitcoinAddress("LLUAaniHSW6eH1QQUrJ7ZAEHurkhx857f3")) == false);
    BOOST_CHECK(oeruShield.IsMasterKey(CBitcoinAddress("LQHK6ejxSbjnu4XKa1XjprjmPhrtPdiJaG")) == false);
}

BOOST_AUTO_TEST_CASE (oerutxout_tests)
{
    std::vector<unsigned char> data = ParseHex("4f455255010000006d1fa8b87567e351717ced5b7c4277cffec6d11f6474b566571759133ae6c4b6d0fc034333f30f2c11bd4f7974531bf42ba87dda9be7b6004d9ed514897133d51850");
    CScript in = CScript() << OP_RETURN << data;
    CTxOut vout(CAmount(0), in);

    COeruTxOut oeruOut(&vout);

    BOOST_CHECK(oeruOut.HasOeruBytes() == true);

    COeruMasterData masterData = oeruOut.GetOeruMasterData();

    BOOST_CHECK(masterData.IsValid() == true);
    BOOST_CHECK(masterData.HasOeruBytes() == true);
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_invalid)
{
    std::vector<unsigned char> data = ParseHex("203fb6ba2a53b41cba97a2ddbaddb52a5b942dd450b6873d8ca633a07eee74820004ce88a1a2eca319a8f02189e82c3ce186d5cc015e60e12e266017ac02c623a8");
    COeruMasterData masterData(data);

    BOOST_CHECK(masterData.IsValid() == false);
}

BOOST_AUTO_TEST_CASE (oerumasterdata_parsing_valid)
{
    CBitcoinAddress signingAddress("LfshT3A5C996TaAPTPBbEAzbc5GKHVX172");
    CBitcoinAddress nonSigningAddress("LczdTJtKRBwctPYzwaJ5zsZKxeDcw8dUcv");
    CBitcoinAddress minerAddress("LcnEALjRU67rvRNUQqnTeujvFwqQAUJTxA");

    std::vector<unsigned char> data = ParseHex("4f45525501000e178520495f81e98372a50b7ba6121182aef28d4f104c1bd260e2b8e0924e670081d5d208ab1f6b7cde0cc18b8d1011aea197314aff0b09bd007f5bfec22520e81603c4");
    COeruMasterData masterData(data);

    BOOST_CHECK(masterData.IsValid() == true);
    BOOST_CHECK(masterData.HasOeruBytes() == true);

    bool enable;
    BOOST_CHECK(masterData.GetEnable(enable) == true);
    BOOST_CHECK(enable == true);

    uint64_t height;
    BOOST_CHECK(masterData.GetHeight(height) == true);
    BOOST_CHECK(height == 923525);

    std::vector<unsigned char> sig;
    BOOST_CHECK(masterData.GetSignature(sig) == true);
    BOOST_CHECK(sig.size() == 65);

    std::string rawMessage;
    BOOST_CHECK(masterData.GetRawMessage(rawMessage) == true);
    BOOST_CHECK(rawMessage == "01000e1785");

    BOOST_CHECK(masterData.HasValidSignature(signingAddress, minerAddress) == true);
    BOOST_CHECK(masterData.HasValidSignature(nonSigningAddress, minerAddress) == false);
}

BOOST_AUTO_TEST_CASE (oerusignature_checker)
{
    CBitcoinAddress addr1("LaZ27rggR2KnmvVGxa3kzkoqxgDYidti2k");
    CBitcoinAddress addr2("LgEHSpv22knkaSR1ZbPSaxqtXujReQykK9");
    CBitcoinAddress addr3("LaZ27rggR2KnmvVGxa3kzkoqxgDYidti2k");
    CBitcoinAddress addr4("LfshT3A5C996TaAPTPBbEAzbc5GKHVX172");

    std::string msg1 = "test";
    std::string msg2 = "some-string";
    std::string msg3 = "test1234";
    std::string msg4 = "01000E177CLcnEALjRU67rvRNUQqnTeujvFwqQAUJTxA";

    std::vector<unsigned char> sig1 = ParseHex("1fd18444a624831c6799647edfc061d6462f47540e7f3a96332f57aa1958946d070820bc47f30472487527dcc165591b278ab8f676bede169c2572b1bda39d8a0e");
    std::vector<unsigned char> sig2 = ParseHex("20e1dbd00c8497433a746c12733c3d3f21e4a993057c50438254b708ce3f5d120a6ee282c1571b5fb4a101501f5291ceb17094aff72dd0fe38947594d2d2e7e766");
    std::vector<unsigned char> sig3 = ParseHex("1f02b3a3e464aafc08a5ac7a0fa66c2be12100375994b708503320d5041e640f4e5ef12b8d84374ddea1f4db504ed0de701b71150a0627e83107d66225c1858be4");
    std::vector<unsigned char> sig4 = ParseHex("1f52b96acc44a72d540cc4004621614b2b29cc7cfd7fc01e3e1cdf6db36680e221693c67e0ff8fcffcafefed460d20a6dd92b5838ab6185b6bb5cb7b9bea52ff15");

    CSignatureChecker sigChecker;

    BOOST_CHECK(sigChecker.VerifySignature(msg1, sig1, addr1) == true);
    BOOST_CHECK(sigChecker.VerifySignature(msg2, sig2, addr2) == true);
    BOOST_CHECK(sigChecker.VerifySignature(msg3, sig3, addr3) == true);
    BOOST_CHECK(sigChecker.VerifySignature(msg4, sig4, addr4) == true);

    BOOST_CHECK(sigChecker.VerifySignature(msg1, sig1, addr2) == false);
    BOOST_CHECK(sigChecker.VerifySignature(msg2, sig2, addr1) == false);
}

BOOST_AUTO_TEST_SUITE_END()
