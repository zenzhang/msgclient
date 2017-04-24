//HttpServer.h

#ifndef      _STREAMSDK_HTTP_MANAGER_
#define      _STREAMSDK_HTTP_MANAGER_

#include <util/protocol/http/HttpProxyManager.h>

namespace streamsdk
{
    class HttpSession;

    class HttpManager
       : public util::protocol::HttpProxyManager<HttpSession, HttpManager>
    {
    public:
        HttpManager(boost::asio::io_service & io_svc);
        ~HttpManager();
        virtual boost::system::error_code startup();
        virtual void shutdown();
        

        boost::asio::io_service & get_io_svc();
    private:
        boost::asio::io_service &io_svc_;
        framework::network::NetName addr_;
    };

} // namespace streamsdk

#endif
