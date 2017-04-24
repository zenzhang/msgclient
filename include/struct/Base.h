#ifndef _PROTOCOL_BASH_H_
#define _PROTOCOL_BASH_H_

#include <framework/network/Endpoint.h>

namespace protocol
{
	struct SocketAddr
	{
		boost::uint32_t IP;
		boost::uint16_t Port;

		SocketAddr(
			boost::uint32_t ip = 0,
			boost::uint16_t port = 0)
			: IP(ip)
			, Port(port)
		{
		}

		explicit SocketAddr(
			const boost::asio::ip::udp::endpoint& end_point)
		{
			IP = end_point.address().to_v4().to_ulong();
			Port = end_point.port();
		}

		bool operator <(
			const SocketAddr& socket_addr) const
		{
			if (IP != socket_addr.IP)
				return IP < socket_addr.IP;
			else
				return Port < socket_addr.Port;
		}

		friend bool operator == (
			const SocketAddr & l,
			const SocketAddr & r)
		{
			return l.IP == r.IP && l.Port == r.Port;
		}

		friend bool operator != (
			const SocketAddr & l,
			const SocketAddr & r)
		{
			return !(l == r);
		}

		friend std::ostream& operator << (
			std::ostream& os,
			const SocketAddr& socket_addr)
		{
			boost::asio::ip::udp::endpoint ep = framework::network::Endpoint(socket_addr.IP, socket_addr.Port);
			return os << ep.address().to_string() << ":" << ep.port();
		}
	};
#pragma pack(push, 1)
	struct CandidatePeerInfo
	{
        boost::uint16_t protocol_version_;
		boost::uint32_t internal_ip_;
		boost::uint16_t internal_port_;
		boost::uint32_t external_ip_;
		boost::uint16_t external_port_;
        boost::uint32_t stun_ip_;
		boost::uint16_t stun_port_;
		boost::uint8_t  nat_type_;

		template <typename Archive>
		void serialize(Archive & ar)
		{
            ar & protocol_version_;
            ar & internal_ip_;
            ar & internal_port_;
            ar & external_ip_;
            ar & external_port_;
            ar & stun_ip_;
            ar & stun_port_;
            ar & nat_type_;
		}

		static boost::uint32_t length()
		{
            return sizeof(boost::uint16_t) + sizeof(boost::uint32_t) + sizeof(boost::uint16_t) + sizeof(boost::uint32_t) +
                sizeof(boost::uint16_t) + sizeof(boost::uint32_t) + sizeof(boost::uint16_t) + sizeof(boost::uint8_t);
		}

		CandidatePeerInfo()
            : protocol_version_(0)
            , internal_ip_(0)
            , internal_port_(0)
            , external_ip_(0)
            , external_port_(0)
            , stun_ip_(0)
            , stun_port_(0)
            , nat_type_(255)
		{

		}

		CandidatePeerInfo(
            boost::uint16_t protocol_version,
			boost::uint32_t internal_ip,
			boost::uint16_t internal_port,
			boost::uint32_t external_ip,
			boost::uint16_t external_port,
			boost::uint32_t stun_ip,
			boost::uint16_t stun_port,
            boost::uint8_t nat_type)
            : protocol_version_(protocol_version)
            , internal_ip_(internal_ip)
            , internal_port_(internal_port)
            , external_ip_(external_ip)
            , external_port_(external_port)
            , stun_ip_(stun_ip)
            , stun_port_(stun_port)
            , nat_type_(nat_type)
		{
		}

		bool NeedStunInvoke(boost::uint32_t LocalDetectedIP) const
		{
			if (LocalDetectedIP == 0 || LocalDetectedIP != external_ip_)
			{
				if (external_ip_ != internal_ip_ && stun_ip_ != 0)
				{
					return true;
				}
			}

			return false;
		}

		bool FromSameSubnet(const protocol::CandidatePeerInfo& peer_info) const
		{
            return external_ip_ != 0 && external_ip_ == peer_info.external_ip_;
		}

		boost::asio::ip::udp::endpoint GetConnectEndPoint(boost::uint32_t LocalDetectedIP) const
		{
			if (external_ip_ == 0)
			{
				return framework::network::Endpoint(internal_ip_, internal_port_);
			}
			else
			{
				if (LocalDetectedIP == external_ip_)
				{
                    return framework::network::Endpoint(internal_ip_, internal_port_);
				}
				else
				{
					return framework::network::Endpoint(external_ip_, external_port_);
				}
			}
		}

		boost::asio::ip::udp::endpoint GetStunEndPoint() const
		{
			return framework::network::Endpoint(stun_ip_, stun_port_);
		}

		boost::asio::ip::udp::endpoint GetLanUdpEndPoint() const
		{
			return framework::network::Endpoint(internal_ip_, internal_port_);
		}

		SocketAddr GetPeerInfo() const
		{
            SocketAddr addr(internal_ip_, internal_port_);
			return addr;
		}

		SocketAddr GetSelfSocketAddr() const
		{
            SocketAddr addr(internal_ip_, internal_port_);
			return addr;
		}

		SocketAddr GetDetectSocketAddr() const
		{
			SocketAddr addr(external_ip_, external_port_);
			return addr;
		}

		SocketAddr GetKeySocketAddr(boost::uint32_t LocalDetectedIP) const
		{
			if (LocalDetectedIP == 0 || LocalDetectedIP != external_ip_)
				return GetDetectSocketAddr();
			else
				return GetSelfSocketAddr();
		}

		SocketAddr GetStunSocketAddr() const
		{
			SocketAddr addr(stun_ip_, stun_port_);
			return addr;
		}

		friend bool operator < (
			const CandidatePeerInfo& l,
			const CandidatePeerInfo& r)
		{
			if (l.GetDetectSocketAddr() != r.GetDetectSocketAddr())
				return l.GetDetectSocketAddr() < r.GetDetectSocketAddr();
			else if (l.GetPeerInfo() != r.GetPeerInfo())
				return l.GetPeerInfo() < r.GetPeerInfo();
			else
				return l.GetStunSocketAddr() < r.GetStunSocketAddr();
		}

		friend bool operator == (
			const CandidatePeerInfo& l, 
			const CandidatePeerInfo& r)
		{
            if (l < r)
            {
                return false;
            }
            else if (r < l)
            {
                return false;
            }

            return true;
		}
	};
#pragma pack(pop)

	inline std::ostream& operator << (std::ostream& os, const CandidatePeerInfo& info)
	{
		return os << "Address: " << framework::network::Endpoint(info.internal_ip_, info.internal_port_).to_string()
			<< ", Detected Address: " << framework::network::Endpoint(info.external_ip_, info.external_port_).to_string()
			<< ", Stun Address: " << framework::network::Endpoint(info.stun_ip_, info.stun_port_).to_string()
			<< ", PeerVersion: " << info.protocol_version_ << ", PeerNatType:"<<short(info.nat_type_);
	}

	enum MY_STUN_NAT_TYPE
	{
		TYPE_ERROR = -1,
		TYPE_FULLCONENAT = 0,
		TYPE_IP_RESTRICTEDNAT,
		TYPE_IP_PORT_RESTRICTEDNAT,
		TYPE_SYMNAT,
		TYPE_PUBLIC
	};
}

#endif  // _PROTOCOL_BASH_H_