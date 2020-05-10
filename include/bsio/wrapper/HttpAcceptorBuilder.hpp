#pragma once

#include <bsio/http/HttpService.hpp>
#include <bsio/wrapper/AcceptorBuilder.hpp>

namespace bsio {

    class HttpAcceptorBuilder : public internal::BaseSocketAcceptorBuilder<HttpAcceptorBuilder>,
                                public internal::BaseSessionBuilder<HttpAcceptorBuilder>
    {
    public:
        HttpAcceptorBuilder& WithEnterCallback(bsio::net::http::HttpSession::EnterCallback callback)
        {
            mEnterCallback = callback;
            return *this;
        }

        HttpAcceptorBuilder& WithParserCallback(bsio::net::http::HttpSession::HttpParserCallback callback)
        {
            mParserCallback = callback;
            return *this;
        }

        HttpAcceptorBuilder& WithWsCallback(bsio::net::http::HttpSession::WsCallback handler)
        {
            mWsCallback = handler;
            return *this;
        }

    protected:
        void beforeStartAccept() override
        {
            BaseSocketAcceptorBuilder<HttpAcceptorBuilder>::mOption.establishHandler =
                [option = internal::BaseSessionBuilder<HttpAcceptorBuilder>::mOption](asio::ip::tcp::socket socket)
            {
                const auto session = TcpSession::Make(std::move(socket),
                    option->recvBufferSize,
                    option->dataHandler,
                    option->closedHandler);
                for (const auto& callback : option->establishHandler)
                {
                    callback(session);
                }
            };

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

            BaseSessionBuilder<HttpAcceptorBuilder>::mOption->dataHandler = dataHandler;
            AddEnterCallback([callback = mEnterCallback, httpSession](TcpSession::Ptr session)
                {
                    httpSession->setSession(session);
                    callback(httpSession);
                });
        }

        void endStartAccept() override
        {
            const auto newOption = std::make_shared<internal::TcpSessionOption>();
            *newOption = *BaseSessionBuilder<HttpAcceptorBuilder>::mOption;
            BaseSessionBuilder<HttpAcceptorBuilder>::mOption = newOption;
        }

    private:
        bsio::net::http::HttpSession::EnterCallback mEnterCallback;
        bsio::net::http::HttpSession::HttpParserCallback mParserCallback;
        bsio::net::http::HttpSession::WsCallback    mWsCallback;
    };

}