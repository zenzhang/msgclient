#ifndef _PEER_UTIL_H
#define _PEER_UTIL_H

#include <framework/system/BytesOrder.h>

#ifdef PEER_PC_CLIENT
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#else
#include "framework/network/Interface.h"
#endif

namespace streamsdk
{
    boost::uint32_t LoadIPs(boost::uint32_t maxCount, boost::uint32_t ipArray[], const hostent& host);

    void LoadLocalIPs(std::vector<boost::uint32_t>& ipArray);

    boost::uint32_t GetLocalFirstIP();
}

#endif