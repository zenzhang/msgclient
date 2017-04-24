#ifndef _PROTOCOL_PACKET_H_
#define _PROTOCOL_PACKET_H_

#include "struct/Base.h"

namespace protocol
{
    const boost::uint16_t PEER_VERSION_V1 = 0x01;
    const boost::uint16_t PEER_VERSION = PEER_VERSION_V1;

    struct Packet
    {
        static boost::uint32_t NewTransactionID()
        {
            static boost::uint32_t glocal_transaction_id = 0;
            glocal_transaction_id ++;
            return glocal_transaction_id;
        }

        Packet()
            : transaction_id_(NewTransactionID())
            , protocol_version_(protocol::PEER_VERSION)
        {
            
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ar & transaction_id_;
            ar & protocol_version_;
        }

        virtual boost::uint32_t length() const
        {
            return 4 + sizeof(transaction_id_) + sizeof(packet_action_) + sizeof(protocol_version_);
        }

        virtual ~Packet(){}

        boost::asio::ip::udp::endpoint end_point_;
        boost::uint8_t packet_action_;
        boost::uint32_t transaction_id_;
        boost::uint16_t protocol_version_;
    };

    template <boost::uint8_t action>
    struct PacketT
    {
        virtual ~PacketT(){}
        static const boost::uint8_t Action = action;
    };

}

#endif  // _PROTOCOL_PACKET_H_
