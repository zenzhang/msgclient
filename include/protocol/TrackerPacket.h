#ifndef _PROTOCOL_TRACKET_PACKET_H_
#define _PROTOCOL_TRACKET_PACKET_H_

#include "protocol/ServerPacket.h"

#include <util/serialization/stl/vector.h>
#include <framework/system/BytesOrder.h>

namespace protocol
{
    //查询guid对应的tracker分组的算法
    inline boost::uint32_t GetGuidMod(const framework::string::Uuid & guid, boost::uint32_t mod)
    {
        boost::uint64_t buf[2];
        memcpy(&buf, &guid.data(), sizeof(guid.data()));
        buf[1] = framework::system::BytesOrder::little_endian_to_host_longlong(buf[1]);
        return static_cast<boost::uint32_t> (buf[1] % mod);
    }


    struct ZQueryPacket
        : public ServerPacketT<0x33>
    {

        ZQueryPacket()
        {

        }

        ZQueryPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t ip,
            boost::uint16_t port)
        {
            transaction_id_ = transaction_id;
            cur_udp_ip_ = ip;
            cur_udp_port_ = port;
            IsRequest = 0;
        }

        ZQueryPacket(
            std::string const & name)
        {
            IsRequest = 1;
            remote_name_ = name;
        }



        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(remote_name_);
            }
            else
            {
                ar & cur_udp_ip_;
                ar & cur_udp_port_;
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest)
            {
                length += remote_name_.size();
            }
            else
            {
                length += sizeof(cur_udp_ip_);
                length += sizeof(cur_udp_port_);
            }
            return length;
        }
        std::string remote_name_;
        boost::uint32_t cur_udp_ip_;
        boost::uint16_t cur_udp_port_;
    };


    struct ZStunPacket
        : public ServerPacketT<0x32>
    {

        ZStunPacket()
        {

        }

        ZStunPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t ip,
            boost::uint16_t port)
        {
            transaction_id_ = transaction_id;
            cur_udp_ip_ = ip;
            cur_udp_port_ = port;
            IsRequest = 0;
        }

        ZStunPacket(
            boost::uint32_t ip,
            boost::uint16_t port)
        {
            IsRequest = 1;
            cur_udp_ip_ = ip;
            cur_udp_port_ = port;
        }



        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            ar & cur_udp_ip_;
            ar & cur_udp_port_;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            length += sizeof(cur_udp_ip_);
            length += sizeof(cur_udp_port_);
            return length;
        }
        boost::uint32_t cur_udp_ip_;
        boost::uint16_t cur_udp_port_;
    };

    struct ZHelloPacket
        : public ServerPacketT<0x31>
    {

        ZHelloPacket()
        {

        }

        ZHelloPacket(
            boost::uint32_t transaction_id)
        {
            transaction_id_ = transaction_id;
            IsRequest = 0;
        }

        ZHelloPacket(std::string const & n)
        {
            IsRequest = 1;
            cur_name = n;
        }



        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(cur_name);
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest)
            {
                length += cur_name.size();
            }
            return length;
        }
        std::string cur_name;
    };
   
    struct ZMessagePacket
        : public ServerPacketT<0x24>
    {

        ZMessagePacket()
        {

        }

        ZMessagePacket(
            boost::uint32_t transaction_id,
            std::string const & from,
            std::string const & to)
        {
            transaction_id_ = transaction_id;
            IsRequest = 0;
            from_name = from;
            to_name = to;
        }

        ZMessagePacket(
            std::string const & from,
            std::string const & to,
            std::string const & msg)
        {
            IsRequest = 1;
            from_name = from;
            to_name = to;
            message = msg;
        }
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(from_name);
                ar & util::serialization::make_sized<boost::uint8_t>(to_name);
                ar & util::serialization::make_sized<boost::uint8_t>(message);
            }
            else
            {
                ar & util::serialization::make_sized<boost::uint8_t>(from_name);
                ar & util::serialization::make_sized<boost::uint8_t>(to_name);
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest)
            {
                length += from_name.size();
                length += to_name.size();
                length += message.size();
            }
            else
            {
                length += from_name.size();
                length += to_name.size();
            }
            return length;
        }
        std::string from_name;
        std::string to_name;
        std::string message;
    };



    struct PeerUser
    {
        PeerUser()
        {

        }
        PeerUser(std::string const& n, 
            boost::uint32_t ui, 
            boost::uint16_t up, 
            boost::uint8_t in)
            : name(n)
            , udp_ip(ui)
            , udp_port(up)
            , is_nat(in)
        {

        }
        std::string name;
        boost::uint32_t udp_ip;
        boost::uint16_t udp_port;
        boost::uint8_t  is_nat;

        boost::uint32_t length() const
        {
            boost::uint32_t len = 0;
            len += name.size();
            len += sizeof(udp_ip);
            len += sizeof(udp_port);
            len += sizeof(is_nat);
            return len;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ar & util::serialization::make_sized<boost::uint8_t>(name);
            ar & udp_ip;
            ar & udp_port;
            ar & is_nat;
        }
    };

    struct ZFriendListPacket
        : public ServerPacketT<0x23>
    {
        ZFriendListPacket()
        {

        }

        ZFriendListPacket(
            std::string const & n)
        {
            IsRequest = 1;
            name = n;
        }


        ZFriendListPacket(
            boost::uint32_t transaction_id)
        {
            transaction_id_ = transaction_id;
            IsRequest = 0;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(name);
            }
            else
            {
                ar & util::serialization::make_sized<boost::uint8_t>(users);
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest)
            {
                length += name.size();
            }
            else
            {
                length += 1;
                for (size_t i = 0; i < users.size(); i++)
                {
                    length += users[i].length();
                }
            }
            return length;
        }
        std::string name;
        std::vector<PeerUser> users;
    };


  

    //online status
    struct ZOnlineStatusPacket
        : public ServerPacketT<0x21>
    {

        ZOnlineStatusPacket()
        {

        }

        ZOnlineStatusPacket(std::string const & n, boost::uint8_t s)
            : name_(n)
            , status_(s)
        {
            IsRequest = 1;
        }

        ZOnlineStatusPacket(
            boost::uint32_t transaction_id)
        {
            transaction_id_ = transaction_id;
            IsRequest = 0;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(name_);
                ar & status_;
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest)
            {
                length += name_.size();
                length += sizeof(status_);
            }
            return length;
        }

        std::string name_;
        //0 上线 1 下线
        boost::uint8_t status_;
    };


    //keep alive
    struct ZKPLPacket
        : public ServerPacketT<0x20>
    {
        ZKPLPacket()
        {}

        ZKPLPacket(std::string const & n, 
            boost::uint8_t nat)
            : name_(n)
            , nat_(nat)
        {
            IsRequest = 1;
        }

        ZKPLPacket(
            boost::uint32_t transaction_id)
        {
            transaction_id_ = transaction_id;
            IsRequest = 0;
        }

            template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest)
            {
                ar & util::serialization::make_sized<boost::uint8_t>(name_);
                ar & nat_;
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (IsRequest) 
            {
                length += name_.size();
            }
            return length;
        }

        std::string name_;
        boost::uint8_t nat_;
    };

    struct ZClientRoutePacket
        : public ServerPacketT<0x10>
    {

        ZClientRoutePacket()
        {
            IsRequest = 1;
        }

        ZClientRoutePacket(
            boost::uint32_t transaction_id
            , boost::uint32_t ip
            , boost::uint16_t udp_port)
        {
            transaction_id_ = transaction_id;
            ip_ = ip;
            udp_port_ = udp_port;
            IsRequest = 0;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (!IsRequest)
            {
                ar & ip_;
                ar & udp_port_;
            }
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t length = ServerPacket::length();
            if (!IsRequest) {
                length += 6;
            }
            return length;
        }


        boost::uint32_t ip_;
        boost::uint16_t udp_port_;
    };

    struct ListPacket
        : public ServerPacketT<0x31>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest) {
                ar & util::serialization::make_sized<boost::uint8_t>(request.rid_);
                ar & request.request_ip_;
                ar & request.nat_type_;
                ar & request.is_tcp_;
            } else{
                ar & util::serialization::make_sized<boost::uint8_t>(response.rid_);
                ar & util::serialization::make_sized<boost::uint16_t>(response.peer_infos_);
            }
        }

        ListPacket()
        {
            // IsRequest = 1;
        }

		virtual boost::uint32_t length() const
		{
			boost::uint32_t length = ServerPacket::length();
			if (IsRequest) {
				length += request.rid_.length() + sizeof(request.nat_type_);
			} else {
				length += response.rid_.length() + sizeof(boost::uint16_t) + response.peer_infos_.size()*CandidatePeerInfo::length();
			}

			return length;
		}

        // request
        ListPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t peer_version,
            const std::string & rid,
            boost::uint32_t request_ip,
            boost::uint8_t nat_type,
            boost::uint8_t is_tcp,
            const boost::asio::ip::udp::endpoint & endpoint
            )
        {
            transaction_id_ = transaction_id;
            protocol_version_ = peer_version;
            request.rid_ = rid;
            request.nat_type_ = nat_type;
            is_tcp = 0;
            end_point_ = endpoint;
            IsRequest = 1;
        }

        ListPacket(
            boost::uint32_t transaction_id,
            const std::string & rid,
            const std::vector<CandidatePeerInfo> & candidate_peer_info,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            response.rid_ = rid;
            response.peer_infos_=candidate_peer_info;
            end_point_ = endpoint;
            IsRequest = 0;
        }

        //这个构造函数需要另外设置peer_infos_
        ListPacket(
            boost::uint32_t transaction_id,
            const std::string& rid,            
            const boost::asio::ip::udp::endpoint& endpoint)
        {
            transaction_id_ = transaction_id;
            response.rid_ = rid;
            end_point_ = endpoint;
            IsRequest = 0;
        }

        struct Request {
            std::string rid_;
            boost::uint32_t request_ip_;
            boost::uint8_t nat_type_;
            boost::uint8_t is_tcp_;
        } request;
        struct Response {
            std::string rid_;
            std::vector<CandidatePeerInfo> peer_infos_;
        } response;
    };

    struct ReportRids
    {
        boost::uint32_t length() const
        {
            boost::uint32_t length = sizeof(boost::uint8_t);
            for (unsigned int i = 0; i < rids_.size(); i++)
            {
                length += rids_[i].length();
            }

            return length;
        }

        ReportRids& operator = (const std::vector<std::string> & rids)
        {
            rids_ = rids;
            return *this;
        }

        std::vector<std::string> rids_;
    };

    template <typename Archive>
    void serialize(Archive & ar, ReportRids & t)
    {
        util::serialization::split_free(ar, t);
    }


    template <typename Archive>
    void load(Archive & ar, ReportRids & t)
    {
        boost::uint8_t rid_count;
        ar & rid_count;
        t.rids_.resize(rid_count);
        for (unsigned int i = 0; i < t.rids_.size(); i++)
        {
            ar & util::serialization::make_sized<boost::uint8_t>(t.rids_[i]);
        }
    };

    template <typename Archive>
    void save(Archive & ar, ReportRids const & t)
    {
        boost::uint8_t rid_count = t.rids_.size();
        ar & rid_count;
        for (unsigned int i = 0; i < t.rids_.size(); i++)
        {
            ar & util::serialization::make_sized<boost::uint8_t>(t.rids_[i]);
        }
    }

    struct ReportPacket
        : public ServerPacketT<0x35>
    {
        ReportPacket()
        {
        }

        // request
        ReportPacket(
            boost::uint32_t transaction_id,
            boost::uint16_t protocol_version,
            const framework::string::Uuid & peer_guid,
            boost::uint8_t is_append,
            boost::uint32_t internal_ip,
            boost::uint16_t internal_port,
            boost::uint32_t stun_ip,
            boost::uint16_t stun_port,
            boost::uint16_t tcp_port,
            boost::uint8_t nat_type,
            const std::vector<std::string> & rids,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            protocol_version_ = protocol_version;
            request.peer_guid_ = peer_guid;
            request.is_append_ = is_append;
            
            request.internal_ip_ = internal_ip;
            request.internal_port_ = internal_port;
            request.stun_ip_ = stun_ip;
            request.stun_port_ = stun_port;
            request.tcp_port_ = tcp_port;
            request.nat_type_ = nat_type;
            request.rids_ = rids;
            end_point_ = endpoint;
            IsRequest = 1;
        }
        // response
        ReportPacket(
            boost::uint32_t transaction_id,
            boost::uint32_t external_ip,
            boost::uint16_t external_port,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            response.external_ip_ = external_ip;
            response.external_port_ = external_port;
            end_point_ = endpoint;
            IsRequest = 0;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            ServerPacket::serialize(ar);
            if (IsRequest){
                ar & request.peer_guid_;
                ar & request.is_append_;
                ar & request.internal_ip_;
                ar & request.internal_port_;
                ar & request.stun_ip_;
                ar & request.stun_port_;
                ar & request.tcp_port_;
                ar & request.nat_type_;
                ar & request.rids_;
            }else{
                ar & response.external_ip_;
                ar & response.external_port_;
            }
        }

		virtual boost::uint32_t length() const
		{
			boost::uint32_t length = ServerPacket::length();
			if (IsRequest){
				length += sizeof(request.peer_guid_);
                length += sizeof(request.is_append_);
                length += sizeof(request.internal_ip_);
				length += sizeof(request.internal_port_);
				length += sizeof(request.stun_ip_);
				length += sizeof(request.stun_port_);
                length += sizeof(request.tcp_port_);
                length += sizeof(request.nat_type_);
                length += request.rids_.length();
			}else{
				length += sizeof(response.external_ip_) + sizeof(response.external_port_);
			}

			return length;
		}		

        struct Request {
            framework::string::Uuid peer_guid_;
            boost::uint8_t is_append_;
            boost::uint32_t internal_ip_;
            boost::uint16_t internal_port_;            
            boost::uint32_t stun_ip_;
            boost::uint16_t stun_port_;
            boost::uint16_t tcp_port_;
            boost::uint8_t nat_type_;
            ReportRids rids_;
        } request;
        struct Response {
            boost::uint32_t external_ip_;
            boost::uint16_t external_port_;
        } response;
    };
}

#endif  // _PROTOCOL_TRACKET_PACKET_H_
