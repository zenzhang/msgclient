#ifndef _PROTOCOL_STUN_SERVER_PACKET_H_
#define _PROTOCOL_STUN_SERVER_PACKET_H_

#include "protocol/ServerPacket.h"

namespace protocol
{
    struct StunKPLPacket
        :public ServerPacketT<0x72>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
        }

        StunKPLPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            protocol_version_ = peer_version;
            end_point_ = endpoint;
            IsRequest = 1;
        }

        StunKPLPacket()
        {
            IsRequest = 1;
        }

    };

    struct StunInvokePacket
        : public ServerPacketT<0x73>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            ar & util::serialization::make_sized<boost::uint8_t>(rid_);
            ar & peer_guid_;
            ar & send_off_time_;
            ar & candidate_peer_info_mine_;
            ar & connect_type_;
            ar & candidate_peer_info_his_;
            ar & ip_pool_size_;
        }

        StunInvokePacket()
        {
            IsRequest = 1;
        }

        StunInvokePacket(
            boost::uint32_t transaction_id,
            const std::string & rid,
            const framework::string::Uuid & peer_guid,
            boost::uint32_t send_off_time,
            const CandidatePeerInfo & peer_info_mine,
            boost::uint8_t connect_type,
            const CandidatePeerInfo & peer_info_his,
            boost::uint16_t ip_pool_size,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            peer_guid_ = peer_guid;
            send_off_time_ = send_off_time;
            candidate_peer_info_mine_ = peer_info_mine;
            connect_type_ = connect_type;
            candidate_peer_info_his_ = peer_info_his;
            ip_pool_size_ = ip_pool_size;
            end_point_ = endpoint;
            IsRequest = 1;
        }

        std::string rid_;
        framework::string::Uuid peer_guid_;
        boost::uint32_t send_off_time_;
        CandidatePeerInfo candidate_peer_info_mine_;
        boost::uint8_t connect_type_;
        CandidatePeerInfo candidate_peer_info_his_;
        boost::uint16_t ip_pool_size_;
    };
}

#endif  // _PROTOCOL_STUN_SERVER_PACKET_H_
