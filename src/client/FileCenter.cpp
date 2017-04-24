//HttpServer.cpp

#include "client/Common.h"
#include "client/FileCenter.h"
#include "client/ServerError.h"
#include "client/util.h"
#include "client/StreamSDK.h"

#include <framework/timer/AsioTimerManager.h>
#include <framework/timer/TickCounter.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::system;

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("FileCenter", 0);

namespace streamsdk
{

    extern framework::timer::TimerQueue & global_second_timer();


    FileCenter::FileCenter()
    {
        try_count_ = 0;
    }

    FileCenter::FileCenter(boost::asio::io_service & io_svc,
        const std::string& name,
        const std::string& serve)
    {
        tast_index_ = 0;
        name_ = name;
        server_ = serve;
        try_count_ = 0;
        step_ = FCInit;
    }
    FileCenter::~FileCenter()
    {

    }
   
    void FileCenter::start()
    {
        timer_.reset(new framework::timer::PeriodicTimer(
            global_second_timer(), 1000, boost::bind(&FileCenter::handle_timer, shared_from_this())));
        timer_->start();
    }

    void FileCenter::stop()
    {
        timer_->cancel();
    }

    void FileCenter::handle_timer()
    {
        if (step_ == FCInit)
            return;

        boost::uint64_t time = framework::timer::TickCounter::tick_count() / 1000 - tick_count_;
        if (time > 2)
        {
            if (++try_count_ > 2)
            {
                //timeout
                step_ = FCInit;
                return;
            }

            if (step_ == FCStunning)
            {
                send_stunpacket();
            }
            else if (step_ == FCQuery)
            {
                send_querypacket();
            }
            tick_count_ = framework::timer::TickCounter::tick_count() / 1000;
        }

    }

    bool FileCenter::add_task(
        const std::string& remote_name, 
        const std::string& file_name,
        boost::uint32_t remote_ip, boost::uint16_t remote_port)
    {
        boost::uint32_t task_id = tast_index_++;
        if (task_)
            return false;
        remote_ip_ = remote_ip;
        remote_port_ = remote_port;
        task_.reset(new FileTask(name_, remote_name, file_name, server_, task_id));
        

        if (remote_ip_ < 1 || remote_port_ < 1)
        {//get ip
            step_ = FCQuery;
            send_querypacket();
        }
        else
        {
            step_ = FCStunning;
            send_stunpacket();
        }
        try_count_ = 0;
        
        return true;
        /*boost::uint32_t task_id = tast_index_++;
        FileTask::p task;s
        task.reset(new FileTask(name_, remote_name, file_name, server_, task_id));
        task->start();
        tast_list_[task_id] = task;
        return task_id;*/
    }


    void FileCenter::send_querypacket()
    {
        assert(step_ == FCQuery);
        protocol::ZQueryPacket packet_send(task_->get_remote_name());
        packet_send.end_point_ = framework::network::Endpoint(server_, 17000);
        the_streamsdk()->DoSendPacket(packet_send);
        tick_count_ = framework::timer::TickCounter::tick_count() / 1000;
    }


    void FileCenter::send_stunpacket()
    {
        assert(step_ == FCStunning);

        {
            protocol::ZHelloPacket packet_send(name_);
            packet_send.end_point_ = framework::network::Endpoint(remote_ip_, remote_port_);
            the_streamsdk()->DoSendPacket(packet_send);
        }

        {
            protocol::ZStunPacket packet_send(remote_ip_, remote_port_);
            packet_send.end_point_ = framework::network::Endpoint(server_, 17000);
            the_streamsdk()->DoSendPacket(packet_send);
        }
        

        tick_count_ = framework::timer::TickCounter::tick_count() / 1000;
    }

    void FileCenter::OnZQueryPacket(protocol::ZQueryPacket const & packet)
    {
        if (step_ == FCQuery)
        {
            remote_ip_ = packet.cur_udp_ip_;
            remote_port_ = packet.cur_udp_port_;

            step_ = FCStunning;
            send_stunpacket();
            try_count_ = 0;
            
        }
    }
    void FileCenter::OnZHelloPacket(protocol::ZHelloPacket const & packet)
    {
        //sender
        if (step_ == FCStunning)
        {
            task_->start();
            step_ = FCInit;
        }

    }
    void FileCenter::OnZStunPacket(protocol::ZStunPacket const & packet)
    {
        protocol::ZHelloPacket packet_send(name_);
        packet_send.end_point_ = framework::network::Endpoint(packet.cur_udp_ip_, packet.cur_udp_port_);
        the_streamsdk()->DoSendPacket(packet_send);
    }

    void FileCenter::OnUdpRecv(
        protocol::Packet const & packet)
    {
        switch (packet.packet_action_)
        {
        case protocol::ZHelloPacket::Action:
        {
            OnZHelloPacket((protocol::ZHelloPacket const &)packet);
        }
        break;
        case protocol::ZStunPacket::Action:
        {
            OnZStunPacket((protocol::ZStunPacket const &)packet);
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

} // namespace streamsdk


