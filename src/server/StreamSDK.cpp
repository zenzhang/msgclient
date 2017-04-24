// StreamSDK.cpp
#include "server/Common.h"
#include "server/StreamSDK.h"
#include "server/HttpManager.h"
#include "server/ServerError.h"

#include <framework/timer/AsioTimerManager.h>

#include <framework/configure/Config.h>
#include <framework/logger/LoggerListRecord.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("StreamSDK", 0);

namespace streamsdk
{
	StreamSDK::StreamSDK()
        : io_svc_(NULL)
        , io_work_(NULL)
        , thread_(NULL)
	{
        static framework::configure::Config testConfig;
        testConfig.profile().set("Logger", "stream_count", "1");
        testConfig.profile().set("LogStream0", "file", "server.log");
        testConfig.profile().set("LogStream0", "append", "false");
        testConfig.profile().set("LogStream0", "level", "5");
        testConfig.profile().set("LogStream0", "size", "0");
        framework::logger::global_logger().load_config(testConfig);
        LOG_S(Logger::kLevelDebug, "StreamSDK::StreamSDK");
	}

	StreamSDK::~StreamSDK() {
		// 
		//this->stop_server();
	}

    static void startup_notify(
        boost::system::error_code & result2,
        boost::mutex & mutex,
        boost::condition_variable & cond,
        boost::system::error_code const & result)
    {
        boost::mutex::scoped_lock lock(mutex);
        result2 = result;
        cond.notify_all();
    }


    boost::system::error_code StreamSDK::startup()
    {
        boost::system::error_code ec;

        do
        {
            boost::uint16_t local_udp_port = 17000;
            user_manager_ = new UserManager(*io_svc_);
            user_manager_->start();

            http_server_ = new HttpManager(*io_svc_);
            ec = http_server_->startup();
            if (ec)
                break;

            udp_server_.reset(new protocol::UdpServer(*io_svc_, shared_from_this()));

            if (false == udp_server_->Listen(local_udp_port))
            {
                ec = error::bind_udp_port_failed;
            }
            else
            {
                RegisterPacket();
                udp_server_->Recv(10);
            }

        } while (false);
        return ec;
    }

    bool StreamSDK::start_server()
    {
        if (NULL != io_svc_)
            return true;

        boost::system::error_code result;
        io_svc_ = new boost::asio::io_service();
        io_work_ = new boost::asio::io_service::work(*io_svc_);
        thread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, io_svc_));

        boost::mutex mutex;
        boost::condition_variable cond;
        boost::mutex::scoped_lock lock(mutex);
        io_svc_->post(boost::bind(startup_notify,
            boost::ref(result),
            boost::ref(mutex),
            boost::ref(cond),
            boost::bind(&StreamSDK::startup, this)));
        cond.wait(lock);
        return (!result);
    }

	void StreamSDK::stop_server()
	{
		
	}

    void StreamSDK::RegisterPacket()
    {
        udp_server_->register_packet<protocol::ZClientRoutePacket>();
        udp_server_->register_packet<protocol::ZKPLPacket>();
        udp_server_->register_packet<protocol::ZOnlineStatusPacket>();
        udp_server_->register_packet<protocol::ZStunPacket>();
        udp_server_->register_packet<protocol::ZFriendListPacket>();
        udp_server_->register_packet<protocol::ZMessagePacket>();
        udp_server_->register_packet<protocol::ZQueryPacket>();
    }
    void StreamSDK::OnRZClientRoutePacket(const protocol::ZClientRoutePacket & packet)
    {
        boost::uint32_t external_ip = packet.end_point_.address().to_v4().to_ulong();
        boost::uint16_t external_port = packet.end_point_.port();

        std::cout << "[OnRZClientRoutePacket] host:" << packet.end_point_.address().to_string() << " port:" << packet.end_point_.port() << std::endl;
        LOG_S(Logger::kLevelEvent, "[OnRZClientRoutePacket] host:" << packet.end_point_.address().to_string() << " port:" << packet.end_point_.port());

        protocol::ZClientRoutePacket packet_send(
            packet.transaction_id_,
            external_ip,
            external_port);
        packet_send.end_point_ = packet.end_point_;
        udp_server_->send_packet(packet_send, packet.protocol_version_);
    }

    UserManager* StreamSDK::UserMgr()
    {
        return user_manager_;
    }

    void StreamSDK::OnUdpRecv(
        protocol::Packet const & packet)
    {
        boost::uint8_t action = packet.packet_action_;
        if (action == protocol::ZClientRoutePacket::Action)
        {
            OnRZClientRoutePacket((protocol::ZClientRoutePacket &)packet);
        }
        else
        {
            user_manager_->OnUdpRecv(packet);
        }
    }
    

    boost::asio::io_service& StreamSDK::io_svc()
    {
        return *io_svc_;
    }

    boost::shared_ptr<StreamSDK> & the_streamsdk()
	{
        static boost::shared_ptr<StreamSDK> the_streamsdk;
        if (NULL == the_streamsdk)
        {
            the_streamsdk.reset(new StreamSDK());
        }
        return the_streamsdk;
	}

    framework::timer::TimerQueue & global_second_timer()
    {
        static framework::timer::AsioTimerManager* timer_manager_1000_ = NULL;
        if (timer_manager_1000_ == NULL)
        {
            timer_manager_1000_ = new framework::timer::AsioTimerManager(the_streamsdk()->io_svc(), boost::posix_time::seconds(1));
            timer_manager_1000_->start();
        }
        return *timer_manager_1000_;
    }

}
