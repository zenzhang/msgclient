// HttpClient.cpp

#include "tools/util_test/Common.h"

#include <util/protocol/http/HttpClient.h>
#include <util/protocol/http/HttpRequest.h>
#include <util/protocol/http/HttpResponse.h>
using namespace util::protocol;

#include <framework/logger/LoggerFormatRecord.h>
#include <framework/network/NetName.h>
using namespace framework::configure;
using namespace framework::logger;
using namespace framework::network;

#include <boost/asio/read.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

using namespace boost::system;
using namespace boost::asio;
using namespace boost::asio::error;
using namespace boost::asio::ip;

streambuf buff;
NetName addr("https://www.baidu.com",443);
HttpRequestHead head;
util::protocol::HttpClient *http_client = NULL;

FRAMEWORK_LOGGER_DECLARE_MODULE("TestHttpClient");

void on_fetch(
    boost::system::error_code const & ec)
{
    http_client->response_head().get_content(std::cout);
    std::string hosts;
    http_client->get_resolvered_host_list(hosts);
    //std::cout << &http_client->response().data() << std::endl;
    delete http_client;
}

void test_http_client_async(io_service &io_svc)
{
    http_client = new util::protocol::HttpClient(io_svc);
    //std::ostream *os = new std::ostream(&buff);
    //http_client->async_open(
    //    "http://jump.g1d.net/%b0%d7%c9%ab%bb%d1%d1%d4(%b5%da001%bc%af).mp4", 
    //    bind(util::binder::ptr_fun(on_open), _1));
    error_code ec;
    //http_client->bind_host("www.sina.com.cn", ec);
    head.method = HttpRequestHead::get;
    head.path = "/public/live/play?program_token=20011945&applt=android&appname=test&appver=1.0.0&platform=android&sessionid=12312313";
    head["Accept"] = "{*/*}";
    head.host = "hpi.video.sina.com.cn";
    head.protocol = "https";
    //head.get_content(std::cout);
    //http://api.sina.cn/sinago/video_location.json?sf_i=4&video_cate=vms&video_id=250884588&video_play_url=http%3A%2F%2Fask.ivideo.sina.com.cn%2Fv_play_ipad.php%3Fvid%3D141240506&fromsinago=1&from=6057093012&preload=0
    //http_client->async_open("http://api.sina.cn/sinago/video_location.json?sf_i=4&video_cate=vms&video_id=250884588&video_play_url=http%3A%2F%2Fask.ivideo.sina.com.cn%2Fv_play_ipad.php%3Fvid%3D141240506&fromsinago=1&from=6057093012&preload=0"
    //    , boost::bind(on_fetch, _1));
    http_client->async_fetch("http://www.baidu.com/a.jpg", boost::bind(on_fetch, _1));
    //http_client->async_fetch("https://chaoshi.tmall.com/tm-chaoshi/pages/index/index?spm=a21bo.50862.201859.3.lQvO4g", 
    //    boost::bind(on_fetch, _1));
    //http_client->async_fetch("http://dispatchadmin.video.sina.com.cn/p2p/bootstrap1", boost::bind(on_fetch, _1));
}

void test_http_client_non_block(io_service &io_svc)
{
    http_client = new util::protocol::HttpClient(io_svc);
    //std::ostream *os = new std::ostream(&buff);
    //http_client->async_open(
    //    "http://jump.g1d.net/%b0%d7%c9%ab%bb%d1%d1%d4(%b5%da001%bc%af).mp4", 
    //    bind(util::binder::ptr_fun(on_open), _1));
    error_code ec;
    http_client->bind_host("jump.g1d.net", ec);
    head.method = HttpRequestHead::get;
    head.path = "/%b0%d7%c9%ab%bb%d1%d1%d4(%b5%da001%bc%af).mp4";
    head["Accept"] = "{*/*,*/*.xml}";
    head.host = "jump.g1d.net";
    std::string host = head["Host"];
    std::string accept = head["Accept"];
    head.get_content(std::cout);
    //http_client->async_open(head, bind(util::binder::ptr_fun(on_open), _1));
    http_client->set_non_block(true, ec);
    http_client->open(head, ec);
    while (ec == would_block) {
        std::cout << "block\n";
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        http_client->is_open(ec);
    }
    http_client->response_head().get_content(std::cout);
    std::string hosts;
    http_client->get_resolvered_host_list(hosts);
    streambuf buf;
    boost::asio::read(*http_client, buf, boost::asio::transfer_all(), ec);
    while (ec == would_block) {
        std::cout << "block\n";
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        boost::asio::read(*http_client, buf, boost::asio::transfer_all(), ec);
    }
    delete http_client;
    std::cout << &buf << std::endl;
}


void test_http_client(Config & conf)
{

    
    //https://hpi.video.sina.com.cn/public/live/play?program_token=20011945&applt=android&appname=test&appver=1.0.0&platform=android&sessionid=12312313

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[test_http_client_async] start"));
        io_service io_svc;
        test_http_client_async(io_svc);
        io_svc.run();
        LOG_F(Logger::kLevelDebug, ("[test_http_client_async] end"));
    }

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[test_http_client_non_block] start"));
        io_service io_svc;
        test_http_client_non_block(io_svc);
        LOG_F(Logger::kLevelDebug, ("[test_http_client_non_block] end"));
    }
}

static TestRegister tr("http_client", test_http_client);
