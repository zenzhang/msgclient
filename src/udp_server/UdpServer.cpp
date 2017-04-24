#include <protocol/UdpServer.h>
#include <util/buffers/SubBuffers.h>
#include <util/buffers/BufferCopy.h>

#include <framework/system/BytesOrder.h>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>

#include "protocol/CheckSum.h"

namespace protocol
{
    UdpServer::UdpServer(
        boost::asio::io_service & io_service,
        IUdpServerListener::p handler)
        : main_io_service_(io_service)
        , handler_(handler)
        , port_(0)
        , minimal_protocol_version_(0)
        , is_running_(false)
    {
        socket_.reset(new boost::asio::ip::udp::socket(main_io_service_));
    }

    bool UdpServer::Listen(
        boost::uint16_t port)
    {
       return Listen("0.0.0.0",port);
    }

    bool UdpServer::Listen(const std::string& ip,boost::uint16_t port)
    {
        boost::system::error_code error;
        socket_->open(boost::asio::ip::udp::v4(), error);
        if (error) {
            return false;
        }

        boost::asio::ip::address_v4 av4;
        av4 = boost::asio::ip::address_v4::from_string(ip, error);

        if (error)
        {
            return false;
        }

        boost::asio::ip::udp::endpoint ep(boost::asio::ip::address(av4),port);

        socket_->bind(ep, error);
        if (error)
        {
            socket_->close(error);
            return false;
        }
        port_ = port;

        is_running_ = true;

        return true;
    }

    bool UdpServer::SetOption(const std::string &ip)
    {
        boost::system::error_code error;
        boost::asio::ip::address_v4 av4;
        av4 = boost::asio::ip::address_v4::from_string(ip,error);
        
        if (error) 
        {
            is_running_ = false;
            return false;
        }
        
        socket_->set_option(boost::asio::ip::multicast::join_group(av4), error);

        if (error)
        {
            is_running_ = false;
            return false;
        }

        return true;
    }

    void UdpServer::Recv(
        boost::uint32_t count)
    {
        for (boost::uint32_t i=0; i < count; ++i)
        {
            Recv();
        }
    }

    void UdpServer::Recv()
    {
        UdpBuffer * recv_buffer = new UdpBuffer;
        if (!recv_buffer) {
            return;
        }

        if (!*recv_buffer) {
            delete recv_buffer;
            recv_buffer = NULL;
            return;
        }

        UdpRecvFrom(recv_buffer);
    }

    void UdpServer::UdpRecvFrom(
        UdpBuffer * recv_buffer)
    {
        socket_->async_receive_from(
            recv_buffer->prepare(),
            recv_buffer->end_point(),
            boost::bind(&UdpServer::HandleUdpRecvFrom,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                recv_buffer));
    }

    void UdpServer::AddCheckSum(boost::asio::const_buffers_3 buffers, boost::uint16_t dest_protocol_version)
    {
        boost::uint32_t & chk_sum = const_cast<boost::uint32_t &>(
            *boost::asio::buffer_cast<boost::uint32_t const *>(*buffers.begin()));

        chk_sum = check_sum_new(util::buffers::sub_buffers(buffers, 4));
    }

    void UdpServer::UdpSendTo(
        const UdpBuffer & send_buffer,
        boost::uint16_t dest_protocol_version)
    {
        AddCheckSum(send_buffer.data(), dest_protocol_version);

        boost::system::error_code ec;
        socket_->send_to(send_buffer.data(), send_buffer.end_point(), 0, ec);
    }

    void UdpServer::UdpAsyncSendTo(
        boost::shared_ptr<UdpBuffer> send_buffer,
        boost::uint16_t dest_protocol_version)
    {
        AddCheckSum(send_buffer->data(), dest_protocol_version);

        socket_->async_send_to(send_buffer->data(), send_buffer->end_point(),
            boost::bind(&UdpServer::HandleUdpSendTo, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            send_buffer));
    }

    void UdpServer::HandleUdpSendTo(const boost::system::error_code & error,
        boost::uint32_t bytes_transferred, boost::shared_ptr<UdpBuffer> send_buffer)
    {
    }

    void UdpServer::HandleUdpRecvFrom(
        boost::system::error_code const & error,
        boost::uint32_t bytes_transferred,
        UdpBuffer * recv_buffer)
    {
        if (!handler_)
        {
            delete recv_buffer;
            return;
        }

        Recv();

        if (!error && bytes_transferred > sizeof(boost::uint32_t) + sizeof(boost::uint8_t))
        {
            recv_buffer->commit(bytes_transferred);
            boost::uint32_t chk_sum;
            IUdpBufferStream is(recv_buffer);
            is.read((char *)&chk_sum, sizeof(boost::uint32_t));

            boost::uint8_t action = is.get();
            std::map<boost::uint8_t, packet_handler_type>::const_iterator iter =
                packet_handlers_.find(action);
            if (iter != packet_handlers_.end())
            {
                boost::uint16_t protocol_version;
                if (get_protocol_version(recv_buffer, bytes_transferred-5, action, protocol_version))
                {
                    is.unget(); // 将action放回Buffer中
                    if (verify_check_sum(recv_buffer, chk_sum, protocol_version)) 
                    {
                        is.get(); // 将action从Buffer中读出，下面的handle函数只序列化action后面的字段
                        (this->*iter->second)(recv_buffer);
                        return;
                    }
                }                
            }
        }

        delete recv_buffer;
    }

    bool UdpServer::get_protocol_version(UdpBuffer * buffer, boost::uint32_t bytes_left,
        boost::uint8_t action, boost::uint16_t & protocol_version)
    {
        if (bytes_left < 4)
        {
            return false;
        }

        boost::uint16_t * version = (boost::uint16_t *)(buffer->GetHeadBuffer()+9);
        protocol_version = *version;
        return true;
    }

    bool UdpServer::verify_check_sum(UdpBuffer * buffer, boost::uint32_t chk_sum,
        boost::uint16_t protocol_version)
    {
        if (protocol_version >= minimal_protocol_version_)
        {
            return (check_sum_new(buffer->data()) == chk_sum);
        }
        else
        {
            return false;
        }
    }

    void UdpServer::Close()
    {
        is_running_ = false;

        boost::system::error_code error;
        socket_->close(error);
        handler_.reset();
        packet_handlers_.clear();
    }
}
