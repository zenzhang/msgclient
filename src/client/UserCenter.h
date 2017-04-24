//UserCenter.h

#ifndef      _STREAMSDK_USER_CENTER_
#define      _STREAMSDK_USER_CENTER_

#include <protocol/UdpServer.h>

#include <framework/timer/Timer.h>

namespace streamsdk
{
    class UserCenter
        : public boost::enable_shared_from_this<UserCenter>
    {
    public:
        UserCenter();
        UserCenter(
            boost::asio::io_service & io_svc, 
            const std::string& name,
            const std::string& server);
        virtual ~UserCenter();

        void OnUdpRecv(
            protocol::Packet const & packet);

        void send_heartbeat();

        void start();
        void stop();

        std::string name();
        std::string server();

        bool kpl_is_ok();

    private:
        void OnZClientRoutePacket(protocol::ZClientRoutePacket const & packet);
        void OnZKPLPacket(protocol::ZKPLPacket const & packet);
    private:
        void handle_timer_kpl();
        void handle_timer_timeout();


        void start_check();
    private:
        boost::shared_ptr<framework::timer::Timer> timer_kpl_;
        boost::shared_ptr<framework::timer::Timer> timer_timeout_;

        boost::uint16_t local_port_;
        boost::uint8_t is_nat_;
        std::string name_;
        std::string server_;
        boost::uint32_t try_count_;
        boost::uint64_t tick_count_;
        bool kpl_is_ok_;
    };

} // namespace streamsdk

#endif
