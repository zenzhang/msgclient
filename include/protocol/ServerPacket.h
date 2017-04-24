#ifndef _PROTOCOL_SERVER_PACKET_H_
#define _PROTOCOL_SERVER_PACKET_H_

#include "protocol/Packet.h"

namespace protocol
{

    struct ServerPacket
        : Packet
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & IsRequest;
        }

        ServerPacket()
        {
            IsRequest = 1;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = Packet::length();
            length += sizeof(IsRequest);
            return length;
        }

        boost::uint8_t IsRequest;
    };

    template <boost::uint8_t action>
    struct ServerPacketT
        : PacketT<action>
        , ServerPacket
    {
    };

}

#endif  // _PROTOCOL_SERVER_PACKET_H_
