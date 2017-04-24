#ifndef _PROTOCOL_PEER_PACKET_H_
#define _PROTOCOL_PEER_PACKET_H_

#include "protocol/Packet.h"
#include "struct/Base.h"
#include "struct/UdpBuffer.h"

#include <util/serialization/stl/string.h>
#include <util/serialization/stl/vector.h>
#include <util/archive/ArchiveBuffer.h>

namespace protocol
{
    struct CommonPeerPacket
        : Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & util::serialization::make_sized<boost::uint8_t>(rid_);
        }

        CommonPeerPacket()
        {
        }

        virtual ~CommonPeerPacket(){}

        virtual boost::uint32_t length() const
        {
            return Packet::length() + sizeof(boost::uint8_t) + rid_.length();
        }

        friend bool operator == (CommonPeerPacket const & l, CommonPeerPacket const & r)
        {
            return l.rid_ == r.rid_;
        }

        std::string rid_;
    };

    template <boost::uint8_t action>
    struct CommonPeerPacketT
        : PacketT<action>
        , CommonPeerPacket
    {
        virtual ~CommonPeerPacketT(){}
    };

    enum ConnectType
    {
        CONNECT_VOD             = 0,
        CONNECT_LIVE_PEER       = 1,
        CONNECT_MAX             = 2
    };

    /**
    *@brief  Error 包
    */
    struct ErrorPacket
        : CommonPeerPacketT<0x51>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);
            ar & error_code_;
        }

        template <typename PeerPacketType>
        ErrorPacket(PeerPacketType const &packet)
        {
            transaction_id_ = packet.transaction_id_;
            rid_ = packet.rid_;
            end_point_ = packet.end_point_;
            // PacketAction = Action;
        }

        ErrorPacket(boost::uint32_t transaction_id, const std::string & rid,
            boost::uint16_t error_code, const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            error_code_ = error_code;
            end_point_ = endpoint;
        }

        ErrorPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length() + sizeof(error_code_);
        }

        static const boost::uint16_t EXCHANGE_NO_RESOURCEID = 0x0011;
        static const boost::uint16_t EXCHANGE_NOT_DOWNLOADING = 0x0012;
        static const boost::uint16_t CONNECT_NO_RESOURCEID = 0x0021;
        static const boost::uint16_t CONNECT_CONNECTION_FULL = 0x0022;
        static const boost::uint16_t ANNOUCE_NO_RESOURCEID = 0x0031;
        static const boost::uint16_t SUBPIECE_NO_RESOURCEID = 0x0041;
        static const boost::uint16_t SUBPIECE_SUBPIECE_NOT_FOUND = 0x0042;
        static const boost::uint16_t SUBPIECE_UPLOAD_BUSY = 0x0043;
        static const boost::uint16_t RIDINFO_NO_RESOURCEID = 0x0051;

        static const boost::uint16_t SN_BUSY = 0x60;
        static const boost::uint16_t SN_ERROR = 0x61;

        boost::uint16_t error_code_;
    };

    /**
    *@brief  Connect 包
    */
    struct ConnectPacket
        : CommonPeerPacketT<0x52>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);
            ar & basic_info_;
            ar & send_off_time_;
            ar & peer_info_;
            ar & connect_type_;
            ar & ip_pool_size_;
        }

        ConnectPacket(boost::uint32_t transaction_id,
            const std::string & rid, boost::uint16_t protocol_version,
            boost::uint8_t basic_info, boost::uint32_t send_off_time,
            const CandidatePeerInfo & peer_info, boost::uint8_t connect_type,
            const boost::asio::ip::udp::endpoint & endpoint, boost::uint16_t ip_pool_size = 0)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            protocol_version_ = protocol_version;
            basic_info_ = basic_info;
            send_off_time_ = send_off_time;
            peer_info_ = peer_info;            assert(connect_type < CONNECT_MAX);
            connect_type_ = connect_type;
            end_point_ = endpoint;
            ip_pool_size_ = ip_pool_size;
        }

        ConnectPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length() + sizeof(basic_info_) + sizeof(send_off_time_)
                + sizeof(peer_info_) + sizeof(connect_type_) + sizeof(ip_pool_size_);
        }

        inline bool IsRequest() const{ if (basic_info_ % 2 == 0) return true; return false;}

        boost::uint8_t basic_info_;
        boost::uint32_t send_off_time_;
        CandidatePeerInfo peer_info_;
        boost::uint8_t connect_type_;
        boost::uint16_t ip_pool_size_;
    };

    /**
    *@brief  PeerExchange 包
    */
    struct PeerExchangePacket
        : CommonPeerPacketT<0x57>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);
            ar & basic_info_;
            ar & util::serialization::make_sized<boost::uint8_t>(peer_info_);
        }

        PeerExchangePacket()
        {
        }

        PeerExchangePacket(
            boost::uint32_t transaction_id,
            const std::string & rid,
            boost::uint8_t basic_info,
            std::vector<CandidatePeerInfo> & candidate_peer_info,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            basic_info_ = basic_info;
            peer_info_.swap(candidate_peer_info);
            end_point_ = endpoint;
            // PacketAction = Action;
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length() + sizeof(basic_info_) + peer_info_.size() * sizeof(CandidatePeerInfo);
        }

        inline bool IsRequest() const{ if (basic_info_ % 2 == 0) return true; return false;}

        boost::uint8_t basic_info_;
        std::vector<CandidatePeerInfo> peer_info_;
    };

    /**
    * CloseSessionPacket
    */
    struct CloseSessionPacket
        : Packet
        , PacketT<0x5C>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
        }

        CloseSessionPacket(boost::uint32_t transaction_id, boost::uint16_t protocol_version,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            protocol_version_ = protocol_version;
            end_point_ = endpoint;
        }

        CloseSessionPacket()
        {
        }

        virtual boost::uint32_t length() const
        {
            return Packet::length();
        }
    };
}

#endif  // _PROTOCOL_PEER_PACKET_H_