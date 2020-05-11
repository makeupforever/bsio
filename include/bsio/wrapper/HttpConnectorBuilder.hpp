#pragma once

#include <bsio/wrapper/ConnectorBuilder.hpp>
#include <bsio/wrapper/internal/HttpSessionBuilder.hpp>

namespace bsio {

    class HttpConnectionBuilder : public internal::BaseTcpSessionConnectBuilder<HttpConnectionBuilder, internal::BaseHttpBuilder<HttpConnectionBuilder>>
    {
    private:
        void beforeAsyncConnectOfTcpSessionBuilder() final override
        {
            setupHttp();
        }
    };

}
