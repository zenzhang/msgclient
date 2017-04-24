#ifndef _STREAMSDK_STREAMSDK_H_
#define _STREAMSDK_STREAMSDK_H_

#include "client/UserCenter.h"
#include "client/FileCenter.h"

#include <protocol/UdpServer.h>


namespace streamsdk
{

    class IServerCallback
    {
    public:
        virtual ~IServerCallback()
        {}

        virtual void UpdateFriendList() = 0;
        virtual void OnError(int code) = 0;
        virtual void OnMessageCallback(const char* name, const char* msg) = 0;
        virtual void OnErrorMessageCallback(const char* name, const char* msg) = 0;
    };

    struct Friend
    {
        Friend()
        {
            ip = 0;
            port = 0;
            nat = 1;
        }

        Friend(protocol::PeerUser const & us)
        {
            ip = us.udp_ip;
            port = us.udp_port;
            nat = us.is_nat;
        }

        Friend(std::string const & n)
            : name(n)
        {
            ip = 0;
            port = 0;
            nat = 1;
        }
        template <
            typename Archive
        >
        void serialize(Archive & ar)
        {
            ar  & SERIALIZATION_NVP(name)
                & SERIALIZATION_NVP(ip)
                & SERIALIZATION_NVP(port)
                & SERIALIZATION_NVP(nat);
        }
        std::string name;
        boost::uint32_t ip;
        boost::uint16_t port;
        boost::uint8_t nat;
    };
    struct FriendList
    {
        boost::uint32_t code;
        std::string     message;
        std::string     server;
        std::vector<Friend> friends;
        template <
            typename Archive
        >
        void serialize(Archive & ar)
        {
            ar  & SERIALIZATION_NVP(code)
                & SERIALIZATION_NVP(message)
                & SERIALIZATION_NVP(server)
                & SERIALIZATION_NVP(friends);
        }
    };


    struct UdpSendCheck
    {
        UdpSendCheck()
            ;
        UdpSendCheck(
            std::string const & from,
            std::string const & to,
            std::string const & msg);



        protocol::ZMessagePacket packet;
        boost::uint64_t time;
        boost::uint32_t try_cout;
    };

    class StreamSDK : 
        public boost::enable_shared_from_this<StreamSDK>,
        public protocol::IUdpServerListener
	{
    public:
		StreamSDK();
		~StreamSDK();

	public:
        bool start_server();
        void stop_server();
        void set_callback_handle(IServerCallback* handle);

        bool login(const std::string& name, std::string& message);
        void logout();

        void send_message(const char* name, const char* msg);

        void send_file(const char* name, const char* file_name);

        void get_friend_list(std::vector<std::string>& list);

        boost::asio::io_service& io_svc();

        void OnUdpRecv(
            protocol::Packet const & packet);

        boost::uint16_t GetLoaclUdpPort();

        template<typename PacketType>
        void DoSendPacket(PacketType const & packet);

        template<typename PacketType>
        void DoSendPacket(PacketType const & packet,
            boost::uint16_t dest_protocol_version);

    private:
        void OnZOnlineStatusPacket(protocol::ZOnlineStatusPacket const & packet);
        void OnZFriendListPacket(protocol::ZFriendListPacket const & packet);
        void OnZMessagePacket(protocol::ZMessagePacket const & packet);
    private:
        boost::system::error_code startup();
        void RegisterPacket();
        void handle_update_friend_list();
        void handle_time_second();
    private:
        boost::asio::io_service* io_svc_;
        boost::asio::io_service::work* io_work_;
        boost::thread* thread_;

        boost::shared_ptr<protocol::UdpServer> udp_server_;
        boost::shared_ptr<UserCenter> user_center_;
        boost::shared_ptr<FileCenter> file_center_;
        std::map<std::string, Friend> friends_;

        IServerCallback* handle_;
        boost::shared_ptr<framework::timer::Timer> timer_friend_list_;
        boost::shared_ptr<framework::timer::Timer> timer_second_;
        boost::mutex mutex_lock_;

        std::list<UdpSendCheck> packet_list_;
	};

    template<typename PacketType>
    void StreamSDK::DoSendPacket(PacketType const & packet)
    {
        udp_server_->send_packet(packet, protocol::PEER_VERSION);
    }

    template<typename PacketType>
    void StreamSDK::DoSendPacket(PacketType const & packet,
        boost::uint16_t dest_protocol_version)
    {
        udp_server_->send_packet(packet, dest_protocol_version);
    }

    boost::shared_ptr<StreamSDK> & the_streamsdk();
}

#endif