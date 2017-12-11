// Copyright (c) 2017 The e-Gulden Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "oerushield/oerusignal.h"

#include <string>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_compat.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "util.h"

/** Reply structure for request_done to fill in */
struct HTTPReply
{
    int status;
    std::string body;
};

COeruSignal *poeruSignalMain = nullptr;

void COeruSignal::InitOeruSignal(std::string strUAComment)
{
    poeruSignalMain = new COeruSignal(strUAComment);
}

COeruSignal::COeruSignal(std::string strUAComment)
{
    this->strUAComment = strUAComment;
}

std::string COeruSignal::CreateSignalPath(int nBlockHeight)
{
    std::string strDelimiter = ":";

    return "/" + this->strUAComment + strDelimiter +
      std::to_string(nBlockHeight) + strDelimiter +
      std::to_string(this->nNextOeruSignalExecutionHeight) + strDelimiter +
      std::to_string(this->tLastRequestTime);
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
        this->nNextOeruSignalExecutionHeight = nBlockHeight + random;

        // Create path before we update the last request time
        // so we can do something with the last request time
        // server side if necessary
        std::string strPath = this->CreateSignalPath(nBlockHeight);

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

        evhttp_add_header(output_headers, "Host", this->hostname.c_str());

        LogPrint("OeruSignal", "Preparing to send signal to path %s\n",
            strPath.c_str());

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
