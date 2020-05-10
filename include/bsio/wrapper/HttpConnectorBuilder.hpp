#pragma once

#include <bsio/http/HttpService.hpp>
#include <bsio/wrapper/ConnectorBuilder.hpp>

namespace bsio {

    class HttpConnectionBuilder : public internal::BaseTcpSessionConnectBuilder<HttpConnectionBuilder>
    {
    public:
        HttpConnectionBuilder& WithEnterCallback(bsio::net::http::HttpSession::EnterCallback callback)
        {
            mEnterCallback = callback;
            return *this;
        }

        HttpConnectionBuilder& WithParserCallback(bsio::net::http::HttpSession::HttpParserCallback callback)
        {
            mParserCallback = callback;
            return *this;
        }

        HttpConnectionBuilder& WithWsCallback(bsio::net::http::HttpSession::WsCallback handler)
        {
            mWsCallback = handler;
            return *this;
        }

    private:
        void beforeAsyncConnectOfTcpSessionBuilder() final override
        {
            auto httpSession = std::make_shared<bsio::net::http::HttpSession>();
            httpSession->setHttpCallback(mParserCallback);
            httpSession->setWSCallback(mWsCallback);

            auto httpParser = std::make_shared<bsio::net::http::HTTPParser>(HTTP_BOTH);
            auto dataHandler = [=](TcpSession::Ptr session, const char* buffer, size_t len) {
                (void)session;
                size_t retlen = 0;

                if (httpParser->isWebSocket())
                {
                    retlen = bsio::net::http::HttpService::ProcessWebSocket(buffer,
                        len,
                        httpParser,
                        httpSession);
                }
                else
                {
                    retlen = bsio::net::http::HttpService::ProcessHttp(buffer,
                        len,
                        httpParser,
                        httpSession);
                }

                return retlen;
            };

            BaseSessionBuilder<HttpConnectionBuilder>::mOption->dataHandler = dataHandler;
            AddEnterCallback([callback = mEnterCallback, httpSession](TcpSession::Ptr session)
                {
                    httpSession->setSession(session);
                    callback(httpSession);
                });
        }

    private:
        bsio::net::http::HttpSession::EnterCallback mEnterCallback;
        bsio::net::http::HttpSession::HttpParserCallback mParserCallback;
        bsio::net::http::HttpSession::WsCallback    mWsCallback;
    };

}
