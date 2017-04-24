//HttpServer.cpp

#include "server/Common.h"
#include "server/HttpSession.h"
#include "server/HttpManager.h"
#include "server/ServerError.h"
#include "server/StreamSDK.h"

#include <framework/string/Url.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>
using namespace boost::system;

#include <vector>
FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("HttpSession", 0);

namespace streamsdk
{
#ifdef WIN32
#define reponse_format "{\"code\": %d,\"message\" : \"%s\",\"server\" : \"127.0.0.1\",\"friends\": [%s] }"
#else
#define reponse_format "{\"code\": %d,\"message\" : \"%s\",\"server\" : \"120.92.160.186\",\"friends\": [%s] }"
#endif
//reponse_user_format(name,ip,port,nat)
#define reponse_user_format "{\"name\": \"%s\",\"ip\" : %ld,\"port\": %d,\"nat\":%d}"

    HttpSession::HttpSession(
        HttpManager & mgr)
        : util::protocol::HttpProxy(mgr.get_io_svc())
    {
    }

    HttpSession::~HttpSession()
    {
        HttpSession::Close();
    }

    bool HttpSession::on_receive_request_head(util::protocol::HttpRequestHead & request_head)
    {
        return false;
    }

    void HttpSession::local_process(response_type const & resp)
    {

        get_request_head().get_content(std::cout);
        error_code ec;
        std::string tmphost = "http://host";
        std::string name;
        framework::string::Url url(tmphost + get_request_head().path);
        LOG_S(Logger::kLevelEvent, "[local_process] path:" << get_request_head().path);
        url.decode();
        name = url.param("name");
        if (url.path().size() > 1)
            option_ = url.path().substr(1);

        if ("login" == option_)
        {
            if (name.empty())
            {
                ec = error::not_name_param;
            }
            else
            {
               bool ret = the_streamsdk()->UserMgr()->add_user(name);
               if (!ret)
                   ec = error::user_exist_before;
            }
        }
        else if ("logout" == option_)
        {
            if (name.empty())
            {
                ec = error::not_name_param;
            }
            else
            {
                the_streamsdk()->UserMgr()->remove_user(name);
            }
        }
        else
        {
            ec = error::support_method;
        }
        make_error_response_body(body_, ec);
        ec.clear();
        resp(ec, body_.size());
    }

    void HttpSession::transfer_response_data(response_type const & resp)
    {
        get_response_head().get_content(std::cout);
        LOG_S(Logger::kLevelEvent, "[transfer_response_data] ");
        boost::system::error_code ec;
        size_t tSize = body_.size();
        ec = write(body_);
        body_.clear();
        resp(ec, tSize);
    }

    void HttpSession::on_finish()
    {
        LOG_S(Logger::kLevelEvent, "[on_finish] ");
        HttpSession::Close();
    }
    void HttpSession::on_error(
        boost::system::error_code const & ec)
    {
        LOG_S(Logger::kLevelDebug, "[on_error]  ec:" << ec.message());
        HttpSession::Close();
    }

    void HttpSession::get_user_list(std::string& users)
    {
        users.clear();
        std::map<std::string, UserInfo>& list = the_streamsdk()->UserMgr()->users_list();
        std::map<std::string, UserInfo>::iterator iter = list.begin();
        for (; iter != list.end(); ++iter)
        {
            if (iter != list.begin())
                users += ",";

            char buf[128] = "";
            sprintf(buf, reponse_user_format, iter->first.c_str(),
                iter->second.udp_ip,
                iter->second.udp_port,
                (iter->second.is_nat ? 1 : 0));
            users += buf;
        }
    }


    void HttpSession::make_error_response_body(
        std::string& respone_str,
        boost::system::error_code const & last_error)
    {
        char reoponse[1024] = "";
        get_response_head()["Content-Type"] = "{application/json}";

        if (error::support_method == last_error)
        {
            get_response_head()["Content-Type"] = "{text/html}";
            respone_str = "<html><head>支持的方法:<title>service</title><body><h1>login|logout</h1></body></html>";
        }
        else if (!last_error)
        {
            std::string json_users;
            get_user_list(json_users);
            sprintf(reoponse, reponse_format, last_error.value(), last_error.message().c_str(), json_users.c_str());
            respone_str = reoponse;
            //option_
        }
        else /*if(error::support_method == last_error)*/
        {
            sprintf(reoponse, reponse_format, last_error.value(), last_error.message().c_str(), "");
            respone_str = reoponse;
        }
    }

    boost::system::error_code HttpSession::write(std::string const& msg)
    {
        boost::system::error_code ec;
        boost::asio::write(
            get_client_data_stream(),
            boost::asio::buffer(msg.c_str(), msg.size()),
            boost::asio::transfer_all(),
            ec);

        LOG_S(Logger::kLevelDebug, "[write] msg size:" << msg.size() << " ec:" << ec.message());
        return ec;
    }

    void HttpSession::Close()
    {
        LOG_S(Logger::kLevelEvent, "[Close] ");
    }
} // namespace streamsdk


