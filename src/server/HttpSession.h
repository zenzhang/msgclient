//HttpServer.h

#ifndef      _STREAMSDK_HTTP_SESSION_
#define      _STREAMSDK_HTTP_SESSION_


#include <util/protocol/http/HttpProxyManager.h>
#include <util/protocol/http/HttpProxy.h>
#include <util/protocol/http/HttpRequest.h>
#include <util/protocol/http/HttpResponse.h>

namespace streamsdk
{
    class HttpManager;

    class HttpSession
        : public util::protocol::HttpProxy
    {
    public:
        HttpSession(
            HttpManager & mgr);

        ~HttpSession();

        virtual bool on_receive_request_head(
            util::protocol::HttpRequestHead & request_head);

        virtual void transfer_response_data(
            response_type const & resp);

        virtual void on_error(
            boost::system::error_code const & ec);

        virtual void on_finish();

        virtual void local_process(response_type const & resp);

        void Close();
    private:
        void make_error_response_body(
            std::string& respone_str,
            boost::system::error_code const & last_error);

        boost::system::error_code write(std::string const& msg);

        void get_user_list(std::string& users);
    private:
        std::string body_;
        std::string option_;
    };

} // namespace streamsdk

#endif
