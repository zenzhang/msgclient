//HttpServer.cpp

#include "client/Common.h"
#include "client/UserCenter.h"
#include "client/ServerError.h"
#include "client/util.h"
#include "client/StreamSDK.h"

#include <framework/timer/AsioTimerManager.h>
#include <framework/timer/TickCounter.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::system;

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("UserCenter", 0);

namespace streamsdk
{

    extern framework::timer::TimerQueue & global_second_timer();


    UserCenter::UserCenter()
    {
        kpl_is_ok_ = true;
    }

    UserCenter::UserCenter(boost::asio::io_service & io_svc,
        const std::string& name,
        const std::string& server)
        : timer_kpl_(NULL)
        , timer_timeout_(NULL)
    {
        name_ = name;
        server_ = server;

        try_count_ = 0;
        is_nat_ = 1;

        kpl_is_ok_ = true;
    }
    UserCenter::~UserCenter()
    {

    }

    void UserCenter::start()
    {
        tick_count_ = framework::timer::TickCounter::tick_count() / 1000;
        timer_kpl_.reset(new framework::timer::PeriodicTimer(
            global_second_timer(), 5000, boost::bind(&UserCenter::handle_timer_kpl, shared_from_this())));
        timer_kpl_->start();

        timer_timeout_.reset(new framework::timer::PeriodicTimer(
            global_second_timer(), 5000, boost::bind(&UserCenter::handle_timer_timeout, shared_from_this())));
        start_check();
        send_heartbeat();
    }

    void UserCenter::OnZKPLPacket(protocol::ZKPLPacket const & packet)
    {//更新tickcount
        std::cout << "[UserCenter::OnZKPLPacket]" << std::endl;
        tick_count_ = framework::timer::TickCounter::tick_count() / 1000;
    }

    

    void UserCenter::OnZClientRoutePacket(protocol::ZClientRoutePacket const & packet)
    {
        std::cout << "[UserCenter::OnZClientRoutePacket]" << std::endl;
        timer_timeout_->stop();
        
        boost::uint32_t detect_ip = packet.ip_;
        boost::uint16_t detect_port = packet.udp_port_;
        std::vector<boost::uint32_t> local_ips_;
        LoadLocalIPs(local_ips_);
        if (std::find(local_ips_.begin(), local_ips_.end(), detect_ip) != local_ips_.end() && detect_port == local_port_)
        {
            is_nat_ = 0;
        }
        else
        {
            is_nat_ = 1;
        }
    }

    void UserCenter::OnUdpRecv(
        protocol::Packet const & packet)
    {
        switch (packet.packet_action_)
        {
        case protocol::ZKPLPacket::Action:
        {
            OnZKPLPacket((protocol::ZKPLPacket const &)packet);
        }
        break;
        case protocol::ZClientRoutePacket::Action:
        {
            OnZClientRoutePacket((protocol::ZClientRoutePacket const &)packet);
        }
        break;
        default:
            break;
        }
    }

    std::string UserCenter::name()
    {
        return name_;
    }

    bool UserCenter::kpl_is_ok()
    {
        return kpl_is_ok_;
    }

    std::string UserCenter::server()
    {
        return server_;
    }

    void UserCenter::send_heartbeat()
    {
        protocol::ZKPLPacket packet(name_, is_nat_);
        packet.end_point_ = framework::network::Endpoint(server_, 17000);
        the_streamsdk()->DoSendPacket(packet);
    }

    void UserCenter::stop()
    {
        timer_timeout_->stop();
        timer_kpl_->stop();
    }

    void UserCenter::start_check()
    {
        protocol::ZClientRoutePacket packet;
        packet.end_point_ = framework::network::Endpoint(server_, 17000);
        the_streamsdk()->DoSendPacket(packet);
        local_port_ = the_streamsdk()->GetLoaclUdpPort();
        timer_timeout_->start();
    }

    void UserCenter::handle_timer_kpl()
    {
        boost::uint64_t tick = framework::timer::TickCounter::tick_count() / 1000;
        if (tick - tick_count_ > 15)
        {//和服务器断开
            std::cout << "[send_heartbeat] disconnect with server" << std::endl;
            kpl_is_ok_ = false;
        }
        send_heartbeat();
    }

    void UserCenter::handle_timer_timeout()
    {
        if (try_count_++ > 2)
        {//timeout
            //和服务器连接超时
        }
        else
        {
            start_check();
        }
    }

} // namespace streamsdk


