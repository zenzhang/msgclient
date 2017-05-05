//HttpServer.cpp

#include "server/Common.h"
#include "server/HttpManager.h"
#include "server/HttpSession.h"
#include "server/ServerError.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("HttpManager", 0);

namespace streamsdk
{
    HttpManager::HttpManager(boost::asio::io_service & io_svc)
        : util::protocol::HttpProxyManager<HttpSession, HttpManager>(io_svc)
        , io_svc_(io_svc)
        , addr_("0.0.0.0:8081")
    {

    }

    HttpManager::~HttpManager()
    {

    }

    boost::asio::io_service & HttpManager::get_io_svc()
    {
        return io_svc_;
    }

    boost::system::error_code HttpManager::startup()
    {
        boost::system::error_code ec;
        start(addr_, ec);
        LOG_S(Logger::kLevelEvent, "[startup] host:" << addr_.to_string() << " ec:" << ec.message());
        return ec;
    }

    void HttpManager::shutdown()
    {
        stop();
    }

} // namespace streamsdk


