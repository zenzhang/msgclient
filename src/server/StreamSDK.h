#ifndef _STREAMSDK_STREAMSDK_H_
#define _STREAMSDK_STREAMSDK_H_

#include "server/HttpManager.h"
#include "server/UserManager.h"

#include <protocol/UdpServer.h>

namespace streamsdk
{
    class StreamSDK : public boost::enable_shared_from_this<StreamSDK>,
        public protocol::IUdpServerListener
	{
    public:
		StreamSDK();
		~StreamSDK();

	public:
		bool start_server();
        void stop_server();

        boost::asio::io_service& io_svc();

        void OnUdpRecv(protocol::Packet const & packet);

        UserManager* UserMgr();

 
        template<typename PacketType>
        void DoSendPacket(PacketType const & packet);

        template<typename PacketType>
        void DoSendPacket(PacketType const & packet,
            boost::uint16_t dest_protocol_version);

    private:
        void OnRZClientRoutePacket(const protocol::ZClientRoutePacket & packet);

    private:
        boost::system::error_code startup();
        void RegisterPacket();

    private:
        boost::asio::io_service* io_svc_;
        boost::asio::io_service::work* io_work_;
        boost::thread* thread_;

        boost::shared_ptr<protocol::UdpServer> udp_server_;
        HttpManager* http_server_;
        UserManager* user_manager_;
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