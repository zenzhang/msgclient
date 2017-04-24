//FileCenter.h

#ifndef      _STREAMSDK_FILE_CENTER_
#define      _STREAMSDK_FILE_CENTER_

#include "client/FileTask.h"

#include <protocol/UdpServer.h>
#include <framework/timer/Timer.h>

namespace streamsdk
{
    class FileCenter
        : public boost::enable_shared_from_this<FileCenter>
    {
    public:
        FileCenter();
        FileCenter(
            boost::asio::io_service & io_svc,
            const std::string& name,
            const std::string& server);

        void start();
        void stop();

        bool add_task(const std::string& remote_name, const std::string& file_name, 
            boost::uint32_t remote_ip, boost::uint16_t remote_port);

        virtual ~FileCenter();

        void OnUdpRecv(
            protocol::Packet const & packet);
    private:
        enum FCStep
        {
            FCInit,
            FCQuery,
            FCStunning,
        };
    private:
        void handle_timer();
        void OnZQueryPacket(protocol::ZQueryPacket const & packet);
        void OnZHelloPacket(protocol::ZHelloPacket const & packet);
        void OnZStunPacket(protocol::ZStunPacket const & packet);


        void send_stunpacket();
        void send_querypacket();
    private:
        boost::shared_ptr<framework::timer::Timer> timer_;
        FileTask::p task_;
        boost::uint32_t tast_index_;

        std::string name_;
        std::string server_;
        boost::uint32_t remote_ip_;
        boost::uint16_t remote_port_;
        boost::uint32_t try_count_;
        FCStep step_;
        boost::uint64_t tick_count_;
    };

} // namespace streamsdk

#endif
