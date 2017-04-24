//UserManager.h

#ifndef      _STREAMSDK_USER_MANAGER_
#define      _STREAMSDK_USER_MANAGER_

#include <protocol/UdpServer.h>

namespace framework
{
    namespace timer { class Timer; }
}

namespace streamsdk
{
    struct UserInfo
    {
        UserInfo();
        UserInfo(boost::uint32_t i, boost::uint16_t p);
        boost::uint32_t udp_ip;
        boost::uint16_t udp_port;
        bool is_nat;
        boost::uint64_t tick_count;
    };

    class UserManager
    {
    public:
        UserManager(boost::asio::io_service & io_svc);

        ~UserManager();

        void start();

        void OnUdpRecv(protocol::Packet const & packet);

        bool add_user(const std::string& name);
        bool update_user(const std::string& name, boost::uint32_t ip, boost::uint16_t port, bool nat);
        void remove_user(const std::string name);

        std::map<std::string, UserInfo>& users_list()
        {
            return users_;
        }

    private:
        void handle_timer();
        void OnKeeplivePacket(protocol::ZKPLPacket const & packet);
        void OnZStunPacket(protocol::ZStunPacket const & packet);
        void OnZFriendListPacket(protocol::ZFriendListPacket const & packet);
        void OnZMessagePacket(const protocol::ZMessagePacket & packet);
        void OnZQueryPacket(protocol::ZQueryPacket const & packet);
    private:
        framework::timer::Timer *timer_;
        std::map<std::string, UserInfo> users_;
    };

} // namespace streamsdk

#endif
