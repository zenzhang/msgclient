// StreamSDK.cpp
#include "client/Common.h"
#include "client/StreamSDK.h"
#include "client/ServerError.h"


#include <util/protocol/http/HttpClient.h>

#include <framework/timer/AsioTimerManager.h>

#include <util/serialization/stl/vector.h>
#include <util/archive/JsonIArchive.h> 

#include <framework/timer/TickCounter.h>

#include <framework/configure/Config.h>
#include <framework/logger/LoggerListRecord.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("StreamSDK", 0);

namespace streamsdk
{
    
    framework::timer::TimerQueue & global_second_timer();
#define SERVER_HOST "zenzhang.iok.la"
//#define SERVER_HOST "127.0.0.1"

	StreamSDK::StreamSDK()
        : io_svc_(NULL)
        , io_work_(NULL)
        , thread_(NULL)
	{
        user_center_ = NULL;
        handle_ = NULL;
        static framework::configure::Config testConfig;
        testConfig.profile().set("Logger", "stream_count", "1");
        testConfig.profile().set("LogStream0", "file", "client.log");
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
            boost::uint32_t try_count = 0;
            udp_server_.reset(new protocol::UdpServer(*io_svc_, shared_from_this()));

            while (false == udp_server_->Listen(local_udp_port++))
            {
                if (try_count++ > 10)
                {
                    ec = error::bind_udp_port_failed;
                    break;
                }
                
            }
            if (!ec)
            {
                RegisterPacket();
                udp_server_->Recv(10);
            }

        } while (false);
        return ec;
    }

    void StreamSDK::set_callback_handle(IServerCallback* handle)
    {
        handle_ = handle;
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
        udp_server_->register_packet<protocol::ZFriendListPacket>();
        udp_server_->register_packet<protocol::ZMessagePacket>();
        udp_server_->register_packet<protocol::ZHelloPacket>();
        udp_server_->register_packet<protocol::ZStunPacket>();
        udp_server_->register_packet<protocol::ZQueryPacket>();
    }

    void StreamSDK::OnUdpRecv(protocol::Packet const & packet)
    {
        if (packet.packet_action_ == protocol::ZOnlineStatusPacket::Action)
        {
            OnZOnlineStatusPacket((protocol::ZOnlineStatusPacket const &)packet);
        }
        else if (packet.packet_action_ == protocol::ZFriendListPacket::Action)
        {
            OnZFriendListPacket((protocol::ZFriendListPacket const &)packet);
        }
        else if (packet.packet_action_ == protocol::ZMessagePacket::Action)
        {
            OnZMessagePacket((protocol::ZMessagePacket const &)packet);
        }
        else if (packet.packet_action_ > 0x30 && packet.packet_action_ < 0x40)
        {
            file_center_->OnUdpRecv(packet);
        }
        else
            user_center_->OnUdpRecv(packet);
    }

    void StreamSDK::OnZFriendListPacket(protocol::ZFriendListPacket const & packet)
    {
        boost::mutex::scoped_lock lc(mutex_lock_);
        friends_.clear();
        std::vector<protocol::PeerUser> const & list = packet.users;
        for (size_t i = 0; i < list.size(); i++)
        {
            friends_[list[i].name] = Friend(list[i]);
        }
        handle_->UpdateFriendList();
    }
    void StreamSDK::OnZMessagePacket(protocol::ZMessagePacket const & packet)
    {
        if (packet.IsRequest)
        {
            handle_->OnMessageCallback(packet.from_name.c_str(), packet.message.c_str());
            protocol::ZMessagePacket packet_send(packet.transaction_id_, packet.to_name, packet.from_name);
            packet_send.end_point_ = packet.end_point_;
            DoSendPacket(packet_send);
        }
        else
        {//响应

            std::list<UdpSendCheck>::iterator iter =  packet_list_.begin();
            for (; iter != packet_list_.end(); ++iter)
            {
                if (iter->packet.transaction_id_ == packet.transaction_id_)
                {
                    packet_list_.erase(iter);
                    break;
                }
            }
        }
    }


    void StreamSDK::OnZOnlineStatusPacket(protocol::ZOnlineStatusPacket const & packet)
    {
        std::cout << "[StreamSDK::OnZOnlineStatusPacket] name:" << packet.name_ << " Status:" << (boost::uint32_t)packet .status_<< std::endl;
        boost::mutex::scoped_lock lc(mutex_lock_);
        std::map<std::string, Friend>::iterator iter = friends_.find(packet.name_);
        if (0 == packet.status_) //online
        {
            if (iter == friends_.end())
            {
                friends_[packet.name_] = Friend(packet.name_);
            }
        }
        else if (1 == packet.status_)
        {
            if (iter != friends_.end())
                friends_.erase(iter);
            
        }
        handle_->UpdateFriendList();
        //update UI
    }

    void StreamSDK::get_friend_list(std::vector<std::string>& list)
    {
        boost::mutex::scoped_lock lc(mutex_lock_);
        list.clear();
        std::map<std::string, Friend>::iterator iter = friends_.begin();
        for (; iter != friends_.end(); ++iter)
            list.push_back(iter->first);
    }

    void StreamSDK::handle_update_friend_list()
    {
        protocol::ZFriendListPacket packet(user_center_->name());
        packet.end_point_ = framework::network::Endpoint(user_center_->server(), 17000);
        DoSendPacket(packet);
    }

    bool StreamSDK::login(const std::string& name, std::string& message)
    {
        boost::system::error_code ec;
        logout();


        if (!timer_friend_list_)
        {
            timer_friend_list_.reset(new framework::timer::PeriodicTimer(
                global_second_timer(), 60000, boost::bind(&StreamSDK::handle_update_friend_list, shared_from_this())));

            timer_second_.reset(new framework::timer::PeriodicTimer(
                global_second_timer(), 1000, boost::bind(&StreamSDK::handle_time_second, shared_from_this())));
        }
        

        do
        {
            util::protocol::HttpClient http_client(*io_svc_);
            framework::string::Url url("http://127.0.0.1:8080/login");
            url.host(SERVER_HOST);
            url.param("name", name);
            url.encode();
            http_client.fetch(url, ec);
            if (ec)
            {
                ec = error::connect_server_failed;
                break;
            }   
             
            FriendList frend;
            util::archive::JsonIArchive<> ia(http_client.response_data());
            ia >> frend;
            if (!ia)
            {
                ec = error::connect_server_failed;
                break;
            }

            if (frend.code)
            {
                ec = error::user_exist_before;
                break;
            }

            /*friends_.clear();
            for (size_t i = 0; i < frend.friends.size(); i++)
            {
                Friend& fri = frend.friends[i];
                if (name == fri.name)
                    continue;
                friends_[fri.name] = fri;
            }*/

            file_center_.reset(new FileCenter(*io_svc_, name, frend.server));
            file_center_->start();

            user_center_.reset(new UserCenter(*io_svc_, name, frend.server));
            user_center_->start();
            timer_friend_list_->start();
            timer_second_->start();
            handle_update_friend_list();

        } while (false);
        

        if (ec)
            message = ec.message();

        return (!ec);
    }

    UdpSendCheck::UdpSendCheck()
    {
        time = 0;
        try_cout = 0;
    }

    UdpSendCheck::UdpSendCheck(
        std::string const & from,
        std::string const & to,
        std::string const & msg)
        : packet(from, to, msg)
    {
        time = framework::timer::TickCounter::tick_count() / 1000;
    }

    void StreamSDK::handle_time_second()
    { //std::list<UdpSendCheck> packet_list_;

        if (user_center_ && !user_center_->kpl_is_ok())
        {
            handle_->OnError(1);
            timer_second_->cancel();
            //与服务器断开链接
            return;
        }


        boost::uint64_t time = framework::timer::TickCounter::tick_count() / 1000;
        std::list<UdpSendCheck>::iterator iter = packet_list_.begin();
        for (; iter != packet_list_.end();)
        {
            if ((time - iter->time) > 10)
            {
                handle_->OnErrorMessageCallback(iter->packet.to_name.c_str(), iter->packet.message.c_str());
                packet_list_.erase(iter);
                iter = packet_list_.begin();
                continue;
            }
            else
            {
                break;
            }
            iter++;
        }
    }

    void StreamSDK::send_file(const char* name, const char* file_name)
    {
        if (!user_center_)
            return;


        boost::mutex::scoped_lock lc(mutex_lock_);
        std::map<std::string, Friend>::iterator iter = friends_.find(name);
        if (iter == friends_.end())
            return;

        file_center_->add_task(name, file_name, iter->second.ip, iter->second.port);
    }

    void StreamSDK::send_message(const char* name, const char* msg)
    {
        if (!user_center_)
            return;
        UdpSendCheck packet(user_center_->name(), name, msg);
        packet.packet.end_point_ = framework::network::Endpoint(user_center_->server(), 17000);
        DoSendPacket(packet.packet);
        packet_list_.push_back(packet);
    }


    void StreamSDK::logout()
    {
        
        if (!user_center_)
            return;
        timer_friend_list_->cancel();
        timer_second_->cancel();
        boost::system::error_code ec;
        std::string name = user_center_->name();
        util::protocol::HttpClient http_client(*io_svc_);
        framework::string::Url url("http://127.0.0.1:8080/logout");
        url.host(SERVER_HOST);
        url.param("name", name);
        url.encode();
        http_client.fetch(url, ec);

        user_center_->stop();
        user_center_.reset();

        file_center_->stop();
        file_center_.reset();

    }
    

    boost::asio::io_service& StreamSDK::io_svc()
    {
        return *io_svc_;
    }

    boost::uint16_t StreamSDK::GetLoaclUdpPort()
    {
        return udp_server_->GetUdpPort();
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
