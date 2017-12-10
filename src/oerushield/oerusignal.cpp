// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerusignal.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_compat.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "base58.h"
#include "main.h"
#include "util.h"
#include "utilstrencodings.h"
#include "wallet/wallet.h"

/** Reply structure for request_done to fill in */
struct HTTPReply
{
    int status;
    std::string body;
};

COeruSignal *poeruSignalMain = nullptr;

void COeruSignal::InitOeruSignal(std::string strUAComment)
{
    try
    {
        poeruSignalMain = new COeruSignal(strUAComment);
    }
    catch(const std::invalid_argument &e)
    {
        LogPrint("OeruSignal", "Cannot create poeruSignalMain: %s\n",
                e.what());
    }
}

COeruSignal::COeruSignal(std::string strUAComment)
{
    this->strUAComment = strUAComment;

    if (!pwalletMain)
        throw std::invalid_argument("COeruSignal requires wallet!");

    CBitcoinAddress addr(this->strUAComment);

    if (!addr.IsValid())
        throw std::invalid_argument("uacomment is not a valid e-Gulden address!");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw std::invalid_argument("uacomment is not a valid e-Gulden address!");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw std::invalid_argument("uacomment is not an address owned by us!");

    this->uaSignalKey = &key;
}

std::string COeruSignal::CreateSignalPath(int nBlockHeight)
{
    std::string strMessage = std::to_string(nBlockHeight) + this->strUAComment;

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    std::vector<unsigned char> vchSig;
    if (!this->uaSignalKey->SignCompact(ss.GetHash(), vchSig))
        return "";

    std::string strSignature = EncodeBase64(&vchSig[0], vchSig.size());
    return "/" + strSignature + strMessage;
}

static void http_request_done(struct evhttp_request *req, void *ctx)
{
    // Do nothing
}

bool COeruSignal::ExecuteOeruSignal(int nBlockHeight)
{
    std::time_t now = std::time(nullptr);
    if (now - this->tLastRequestTime < 300)
    {
        return false;
    }

    if (nBlockHeight >= this->nNextOeruSignalExecutionHeight)
    {
        int min = 20;
        int max = 40;
        int random = min + (rand() % static_cast<int>(max - min + 1));
        this->nNextOeruSignalExecutionHeight += random;
        this->tLastRequestTime = now;

        struct event_base *base = event_base_new();
        if (!base) {
            LogPrint("OeruSignal", "cannot create event_base\n");
            return false;
        }

        int port = 80;
        struct evhttp_connection *evcon = evhttp_connection_base_new(base, NULL, this->hostname.c_str(), port);

        if (evcon == NULL) {
            LogPrint("OeruSignal", "create connection failed\n");
            return false;
        }

        HTTPReply response;
        struct evhttp_request *req = evhttp_request_new(http_request_done, (void*)&response);

        if (req == NULL) {
            LogPrint("OeruSignal", "create request failed\n");
            return false;
        }

        struct evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
        assert(output_headers);

        std::string strPath = this->CreateSignalPath(nBlockHeight);
        evhttp_add_header(output_headers, "Host", this->hostname.c_str());

        int r = evhttp_make_request(evcon, req, EVHTTP_REQ_GET, strPath.c_str());
        if (r != 0) {
            evhttp_connection_free(evcon);
            event_base_free(base);

            LogPrint("OeruSignal", "send request failed\n");
            return false;
        }

        LogPrint("OeruSignal", "Sent uasignal successfully!\n");

        event_base_dispatch(base);
        evhttp_connection_free(evcon);
        event_base_free(base);

        return true;
    } else {
        return false;
    }
}
