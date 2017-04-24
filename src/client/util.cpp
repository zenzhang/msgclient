#include "client/Common.h"
#include "client/util.h"
#include <framework/system/BytesOrder.h>

#ifdef BOOST_WINDOWS_API
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#else
#include "framework/network/Interface.h"
#endif

namespace streamsdk
{

    boost::uint32_t LoadIPs(boost::uint32_t maxCount, boost::uint32_t ipArray[], const hostent& host)
    {
        boost::uint32_t count = 0;
        for (int index = 0; host.h_addr_list[index] != NULL; index++)
        {
            if (count >= maxCount)
                break;
            const in_addr* addr = reinterpret_cast<const in_addr*> (host.h_addr_list[index]);
            boost::uint32_t v =
#ifdef BOOST_WINDOWS_API
                addr->S_un.S_addr;
#else
                addr->s_addr;
#endif
            ipArray[count++] = ntohl(v);
        }
        return count;
    }

    boost::uint32_t LoadLocalIPs(boost::uint32_t maxCount, boost::uint32_t ipArray[])
    {
        char hostName[256] = { 0 };
        if (gethostname(hostName, 255) != 0)
        {
            return 0;
        }

        struct hostent* host = gethostbyname(hostName);
        if (host == NULL)
        {
            return 0;
        }
        boost::uint32_t count = LoadIPs(maxCount, ipArray, *host);
        return count;
    }

    void LoadLocalIPs(std::vector<boost::uint32_t>& ipArray)
    {
#ifdef BOOST_WINDOWS_API
        const boost::uint32_t max_count = 32;
        ipArray.clear();
        ipArray.resize(max_count);
        boost::uint32_t count = LoadLocalIPs(max_count, &ipArray[0]);
        assert(count <= max_count);
        if (count < max_count)
        {
            ipArray.resize(count);
        }
#else

        // PPBox获取本地IP的实现
        std::vector<framework::network::Interface> infs;
        enum_interface(infs);
        for (size_t i = 0; i < infs.size(); ++i) {
            framework::network::Interface const & inf = infs[i];

            if (inf.flags & framework::network::Interface::loopback)
                continue;
            if (!(inf.flags & framework::network::Interface::up))
                continue;

            if (inf.addr.is_v4())
            {
                ipArray.push_back(inf.addr.to_v4().to_ulong());
            }
        }
#endif
    }

    boost::uint32_t GetLocalFirstIP()
    {
#ifdef BOOST_WINDOWS_API
        // 使用 ip helper函数
        boost::uint32_t nip = 0;
        PMIB_IPADDRTABLE pIPAddrTable;
        unsigned long dwSize = 0, dwRetVal;

        pIPAddrTable = (MIB_IPADDRTABLE*)malloc(sizeof(MIB_IPADDRTABLE));

        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dwSize variable
        if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
        {
            free(pIPAddrTable);
            pIPAddrTable = (MIB_IPADDRTABLE *)malloc(dwSize);
        }

        // Make a second call to GetIpAddrTable to get the
        // actual data we want
        if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) == NO_ERROR)
        {
            for (boost::uint32_t i = 0; i < pIPAddrTable->dwNumEntries; i++)
            {
                if (pIPAddrTable->table[i].dwAddr != inet_addr("127.0.0.1") && pIPAddrTable->table[i].dwAddr != 0)
                {
                    nip = pIPAddrTable->table[i].dwAddr;
                    break;
                }
            }

        }

        free(pIPAddrTable);

        return ntohl(nip);
#else
        std::vector<framework::network::Interface> interfaces;
        if (::framework::network::enum_interface(interfaces))
            return 0;

        for (boost::uint32_t i = 0; i < interfaces.size(); ++i) {
            framework::network::Interface const & inf = interfaces[i];
            if (string(inf.name) != "lo" && inf.up && inf.addr.is_v4()) {
                return inf.addr.to_v4().to_ulong();
            }
        }

        return 0;
#endif
    }
}