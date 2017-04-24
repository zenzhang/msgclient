#ifndef PROTOCOL_PROTOCOL_H
#define PROTOCOL_PROTOCOL_H

#include <boost/cstdint.hpp>

#include <framework/Framework.h>
#include <util/serialization/Uuid.h>
#include <framework/string/Uuid.h>

const boost::uint32_t LIVE_SUB_PIECE_SIZE = 1280;

#include "struct/Base.h"

#include "protocol/Packet.h"
#include "protocol/PeerPacket.h"
#include "protocol/LivePeerPacket.h"
#include "protocol/StunServerPacket.h"
#include "TrackerPacket.h"

#include "protocol/NatCheckPacket.h"

#endif  // PROTOCOL_PROTOCOL_H
