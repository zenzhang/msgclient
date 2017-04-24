//HttpServer.cpp

#include "server/Common.h"
#include "server/UserManager.h"
#include "server/ServerError.h"
#include "server/StreamSDK.h"

#include <framework/timer/Timer.h>
#include <framework/timer/TickCounter.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("UserManager", 0);

namespace streamsdk
{
#define TIMER_INVETER 5
    extern framework::timer::TimerQueue & global_second_timer();

    UserInfo::UserInfo()
    {
        udp_ip = 0;
        udp_port = 0;
        is_nat = true;
        tick_count = framework::timer::TickCounter::tick_count() / 1000;
    }

    UserInfo::UserInfo(boost::uint32_t i, boost::uint16_t p)
    {
        udp_ip = i;
        udp_port = p;
        is_nat = true;
        tick_count = framework::timer::TickCounter::tick_count()/1000;
    }

    UserManager::UserManager(boost::asio::io_service & io_svc)
        : timer_(NULL)
    {
        
    }

    bool UserManager::add_user(const std::string& name)
    {
        std::map<std::string, UserInfo>::iterator iter = users_.find(name);
        if (iter != users_.end())
            return false;

        for (iter = users_.begin(); iter != users_.end(); ++iter)
        {
            if (iter->second.udp_ip < 1 || iter->second.udp_port < 1)
                continue;
            protocol::ZOnlineStatusPacket packet_send(
                name,
                0);
            packet_send.end_point_ = framework::network::Endpoint(iter->second.udp_ip, iter->second.udp_port);
            the_streamsdk()->DoSendPacket(packet_send);
        }

        users_[name] = UserInfo();
        return true;
    }

    bool UserManager::update_user(const std::string& name, boost::uint32_t ip, boost::uint16_t port, bool nat)
    {
        std::map<std::string, UserInfo>::iterator iter = users_.find(name);
        if (iter == users_.end())
            return false;
        UserInfo& info = iter->second;
        info.udp_ip = ip;
        info.udp_port = port;
        info.is_nat = nat;
        info.tick_count = framework::timer::TickCounter::tick_count() / 1000;
        return true;
    }

    void UserManager::remove_user(const std::string name)
    {
        std::map<std::string, UserInfo>::iterator iter = users_.find(name);
        if (iter == users_.end())
            return;
        users_.erase(iter);

        for (iter = users_.begin(); iter != users_.end(); ++iter)
        {
            if (iter->second.udp_ip < 1 || iter->second.udp_port < 1)
                continue;
            protocol::ZOnlineStatusPacket packet_send(
                name,
                1);
            packet_send.end_point_ = framework::network::Endpoint(iter->second.udp_ip, iter->second.udp_port);
            the_streamsdk()->DoSendPacket(packet_send);
        }
    }

    UserManager::~UserManager()
    {

    }

    void UserManager::OnZStunPacket(protocol::ZStunPacket const & packet)
    {
        
        boost::uint32_t external_ip = packet.end_point_.address().to_v4().to_ulong();
        boost::uint16_t external_port = packet.end_point_.port();

        LOG_S(Logger::kLevelEvent, "[OnZStunPacket] remote ip:" << packet.cur_udp_ip_ << " remote port:" << packet.cur_udp_port_);
        LOG_S(Logger::kLevelEvent, "[OnZStunPacket] send ip:" << external_ip << " send port:" << external_port);

        protocol::ZStunPacket packet_send(packet.transaction_id_, external_ip, external_port);
        packet_send.end_point_ = framework::network::Endpoint(packet.cur_udp_ip_, packet.cur_udp_port_);
        the_streamsdk()->DoSendPacket(packet_send);
    }

    void UserManager::OnKeeplivePacket(protocol::ZKPLPacket const & packet)
    {
        LOG_S(Logger::kLevelEvent, "[OnKeeplivePacket] name:" << packet.name_);
        boost::uint32_t external_ip = packet.end_point_.address().to_v4().to_ulong();
        boost::uint16_t external_port = packet.end_point_.port();
        bool ret = update_user(packet.name_, external_ip, external_port, packet.nat_);
        if (!ret)
        {
            LOG_S(Logger::kLevelEvent, "[OnKeeplivePacket] not exist name:" << packet.name_);
            return;
        }

        protocol::ZKPLPacket packet_send(packet.transaction_id_);
        packet_send.end_point_ = packet.end_point_;
        the_streamsdk()->DoSendPacket(packet_send);
    }

    void UserManager::OnZMessagePacket(const protocol::ZMessagePacket & packet)
    {
        protocol::ZMessagePacket packet_send = packet;
        if (packet.IsRequest)
            std::cout << "Request From:" << packet.from_name << " to:" << packet.to_name << std::endl;
        else
            std::cout << "Response To:" << packet.to_name << std::endl;
        {
            std::cout << "From:" << packet.from_name << " to:" << packet.to_name << std::endl;
            std::map<std::string, UserInfo>::iterator iter = users_.find(packet.to_name);
            if (iter == users_.end())
                return;

            UserInfo& info = iter->second;
            if (info.udp_ip < 1 || info.udp_port < 1)
                return;
            packet_send.end_point_ = framework::network::Endpoint(info.udp_ip, info.udp_port);
            the_streamsdk()->DoSendPacket(packet_send);
        }
    }

    void UserManager::OnZQueryPacket(protocol::ZQueryPacket const & packet)
    {
        LOG_S(Logger::kLevelEvent, "[OnZQueryPacket] name:" << packet.remote_name_);
        std::map<std::string, UserInfo>::iterator iter = users_.find(packet.remote_name_);
        if (iter == users_.end())
            return;

        UserInfo& info = iter->second;
        if (info.udp_ip < 1 || info.udp_port < 1)
            return;

        protocol::ZQueryPacket packet_send(packet.transaction_id_, info.udp_ip, info.udp_port);
        packet_send.end_point_ = packet.end_point_;
        the_streamsdk()->DoSendPacket(packet_send);

    }

    void UserManager::OnZFriendListPacket(protocol::ZFriendListPacket const & packet)
    {
        LOG_S(Logger::kLevelEvent, "[OnZFriendListPacket] name:" << packet.name);
        std::map<std::string, UserInfo>::iterator iter = users_.begin();
        protocol::ZFriendListPacket packet_send(packet.transaction_id_);
        packet_send.end_point_ = packet.end_point_;

        for(; iter != users_.end(); iter++)
        {
            if (iter->first == packet.name)
                continue;
            UserInfo& user = iter->second;
            packet_send.users.push_back(protocol::PeerUser(iter->first, user.udp_ip, user.udp_port, user.is_nat));
        }

        the_streamsdk()->DoSendPacket(packet_send);
    }

    void UserManager::start()
    {
        timer_ = new framework::timer::PeriodicTimer(
            global_second_timer(), 5000, boost::bind(&UserManager::handle_timer, this));
        timer_->start();
    }

    void UserManager::OnUdpRecv(protocol::Packet const & packet)
    {
        switch (packet.packet_action_)
        {
        case protocol::ZKPLPacket::Action:
        {
            OnKeeplivePacket((protocol::ZKPLPacket const &)packet);
        }
        break;
        case protocol::ZStunPacket::Action:
        {
            OnZStunPacket((protocol::ZStunPacket const &)packet);
        }
        break;
        case protocol::ZFriendListPacket::Action:
        {
            OnZFriendListPacket((protocol::ZFriendListPacket const &)packet);
        }
        break;
        case protocol::ZMessagePacket::Action:
        {
            OnZMessagePacket((protocol::ZMessagePacket const &)packet);
        }
        break;
        case protocol::ZQueryPacket::Action:
        {
            OnZQueryPacket((protocol::ZQueryPacket const &)packet);
        }
        break;
        default:
            break;
        }
    }

    void UserManager::handle_timer()
    {
        LOG_S(Logger::kLevelEvent, "[handle_timer] enter");
        boost::uint64_t tick_count = framework::timer::TickCounter::tick_count() / 1000;
        std::map<std::string, UserInfo>::iterator iter = users_.begin();
        while (iter != users_.end())
        {
            if (tick_count - iter->second.tick_count > 3 * TIMER_INVETER)
            {
                LOG_S(Logger::kLevelEvent, "[handle_timer] timeout user:" << iter->first);
                std::cout << "[handle_timer] timeout user:" << iter->first << std::endl;
                remove_user(iter->first);
                iter = users_.begin();
                continue;
            }
            ++iter;
        }

    }
} // namespace streamsdk


