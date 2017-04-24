#ifndef LIVE_PEER_PACKET_H_
#define LIVE_PEER_PACKET_H_

#include "protocol/Packet.h"
#include "struct/LiveSubPieceInfo.h"
#include <util/serialization/stl/vector.h>
#include <boost/dynamic_bitset.hpp>

namespace protocol
{
    const boost::int8_t SUBPIECE_COUNT_IN_ONE_CHECK = 16;

    /**
    *@brief  LiveRequestAnnounce 包
    */
    struct LiveRequestAnnouncePacket
        : CommonPeerPacketT<0xC0>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);

            ar & request_block_id_;
            ar & upload_bandwidth_;
        }

        LiveRequestAnnouncePacket()
        {
            request_block_id_ = 0;
            upload_bandwidth_ = 0;
        }

        LiveRequestAnnouncePacket(boost::uint32_t transaction_id, const std::string & rid, boost::uint32_t request_block_id,
            boost::uint32_t upload_bandwidth, const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            request_block_id_ = request_block_id;
            upload_bandwidth_ = upload_bandwidth;
            end_point_ = endpoint;
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length() + sizeof(request_block_id_) + sizeof(upload_bandwidth_);
        }

        boost::uint32_t request_block_id_;
        boost::uint32_t upload_bandwidth_;
    };

    struct LiveAnnounceMap
    {
        boost::uint16_t block_info_count_;
        boost::uint16_t live_interval_;
        std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> > subpiece_map_;//block_id  每16个subpiece是否有
        std::map<boost::uint32_t, boost::uint16_t> subpiece_nos_; //block_id  有多少个subpiece
        boost::uint32_t request_block_id_;
    };

    template <typename Archive>
    void serialize(Archive & ar, LiveAnnounceMap & t)
    {
        util::serialization::split_free(ar, t);
    }

    template <typename Archive>
    void load(Archive & ar, LiveAnnounceMap & t)
    {
        boost::uint32_t start_block_info_id;

        ar & t.request_block_id_;
        ar & t.block_info_count_;
        std::vector<boost::uint16_t> subpiece_no;
        boost::uint16_t sn;

        
        for (boost::uint16_t i = 0; i < t.block_info_count_; ++i)
        {
            ar & sn;
            subpiece_no.push_back(sn);
        }
        ar & start_block_info_id;
        boost::uint32_t temp_block_info_id = start_block_info_id;

        ar & t.live_interval_;

        boost::uint16_t buffer_map_length;
        ar & buffer_map_length;

        // 首先把网络中传过来的uint8类型的buffer转成01串存在live_bits中
        boost::uint8_t buffer;
        std::deque<bool> live_bits;
        for (boost::uint16_t i = 0; i < buffer_map_length; ++i)
        {
            ar & buffer;
            for (int j = 7; j >= 0; --j)
            {
                live_bits.push_back((buffer >> j) % 2 == 1);
            }
        }

        boost::dynamic_bitset<boost::uint8_t> bit_set;
        t.subpiece_map_.clear();
        for (boost::uint16_t i = 0; i < t.block_info_count_; ++i)
        {
            // 计算这一个block含有多少位01串
            // 第一个subpiece单独占一位，之后每16个subpiece占一位，最后如果不足16个subpiece也占1位
            boost::uint32_t bit_set_num = (subpiece_no[i] + SUBPIECE_COUNT_IN_ONE_CHECK - 2) / SUBPIECE_COUNT_IN_ONE_CHECK + 1;

            assert(live_bits.size() >= bit_set_num);

            bit_set.resize(bit_set_num);

            assert(live_bits.front());

            for (boost::uint32_t j = 0; j < bit_set_num; ++j)
            {
                bit_set[j] = live_bits.front();
                live_bits.pop_front();
            }
            t.subpiece_map_.insert(std::make_pair(temp_block_info_id, bit_set));
            t.subpiece_nos_.insert(std::make_pair(temp_block_info_id, subpiece_no[i]));
            temp_block_info_id += t.live_interval_;
        }

        // 从live_bits中取完后，live_bits应该刚好变空，或者是剩下的都是0
        // 先考虑构造包的时候，假定我们总共有19个校验块（在这里把除第0个subpiece的每16个subpiece称为一个校验块，第0个subpiece自己是一个校验块），
        // 如果live_bits是 10110110 11011011 101，那么转化成uint8_t之后就是182,219,160(最后不足八位的在低位补0)
        // 收到包时，live_bits是10110110 11011011 10100000，因为不知道最后的0是不足补上去的，还是这个校验块不在，所以都存到了live_bits中
        // 那么也就是说live_bits中前19位是我所需要的，从第20位到最后是由于不足八位而补的0
        while (!live_bits.empty())
        {
            assert(live_bits.front() == 0);
            live_bits.pop_front();
        }
    };

    template <typename Archive>
    void save(Archive & ar, LiveAnnounceMap const & t)
    {
        assert(t.subpiece_map_.size() == t.subpiece_nos_.size());

        ar & t.request_block_id_;
        boost::uint16_t block_info_count = t.subpiece_map_.size();
        boost::uint32_t start_block_info_id;
        boost::uint16_t len = 0;
        ar & block_info_count;

        if (0 == block_info_count)
        {
            start_block_info_id = 0;
            ar & start_block_info_id;
            ar & t.live_interval_;
            ar & len;
            return ;
        }

        for (std::map<boost::uint32_t, boost::uint16_t>::const_iterator subpiece_no_iter = t.subpiece_nos_.begin();
            subpiece_no_iter != t.subpiece_nos_.end(); ++subpiece_no_iter)
        {
            ar & subpiece_no_iter->second;
        }

        std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> >::const_iterator iter;
        iter = t.subpiece_map_.begin();
        start_block_info_id = iter->first;
        ar & start_block_info_id;
        ar & t.live_interval_;

        // 计算这些01串转成uint8后有多少位
        std::map<boost::uint32_t, boost::uint16_t>::const_iterator subpiece_no_iter = t.subpiece_nos_.begin();
        for (; iter != t.subpiece_map_.end(); ++iter)
        {
            len += iter->second.size();
            assert(iter->second[0]);
            assert((subpiece_no_iter->second + SUBPIECE_COUNT_IN_ONE_CHECK - 2) / SUBPIECE_COUNT_IN_ONE_CHECK + 1
                == iter->second.size());
            ++subpiece_no_iter;
        }
        len = (len + 7) / 8;
        ar & len;

        boost::dynamic_bitset<boost::uint8_t> buffer;
        int count = 0;
        boost::uint8_t sum = 0;
        for (iter = t.subpiece_map_.begin(); iter != t.subpiece_map_.end(); ++iter)
        {
            buffer = iter->second;

            // 通过左移操作将每8位转成一个uint8类型的
            for (boost::uint32_t i = 0; i < buffer.size(); ++i)
            {
                sum = sum << 1;
                sum += buffer[i];
                count++;
                if (count == 8)
                {
                    ar & sum;
                    sum = 0;
                    count = 0;
                }
            }
        }

        // 最后不足8位的左移相应的位数，也序列化成一个uint8类型的
        if (count != 0)
        {
            sum = sum << (8 - count);
            ar & sum;
        }
    }

     /**
    *@brief  LiveAnnounce 包
    */
    struct LiveAnnouncePacket
        : CommonPeerPacketT<0xC1>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);
            ar & live_announce_map_;
        }

        LiveAnnouncePacket(boost::uint32_t transaction_id, const std::string & rid,
            boost::uint16_t block_info_count,
            const std::map<boost::uint32_t, boost::dynamic_bitset<boost::uint8_t> > & subpiece_map,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            live_announce_map_.block_info_count_ = block_info_count;
            live_announce_map_.subpiece_map_ = subpiece_map;
            end_point_ = endpoint;
        }

        LiveAnnouncePacket(boost::uint32_t transaction_id, const std::string & rid,
            const LiveAnnounceMap & live_announce_map,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            live_announce_map_ = live_announce_map;
            end_point_ = endpoint;
        }

        LiveAnnouncePacket()
        {
            live_announce_map_.block_info_count_ = 0;
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length()
                + sizeof(boost::uint32_t)  // request_block_id_
                + sizeof(boost::uint16_t)  // piece_info_count_
                + sizeof(boost::uint16_t) * live_announce_map_.subpiece_map_.size()  // subpiece_no_
                + sizeof(boost::uint32_t)  // start_piece_info_id_
                + sizeof(boost::uint16_t)  // live_interval_
                + sizeof(boost::uint16_t)  // len
                + live_announce_map_.subpiece_map_.size() * sizeof(boost::uint8_t);
        }

        LiveAnnounceMap live_announce_map_;
    };

    /**
    *@brief  LiveRequestSubPiecePacket
    */
    struct LiveRequestSubPiecePacket
        : CommonPeerPacketT<0xC2>
    {
        template <typename Archive>
        void serialize(Archive & ar)
        {
            CommonPeerPacket::serialize(ar);

            ar & util::serialization::make_sized<boost::uint8_t>(sub_piece_infos_);
            request_sub_piece_count_ = sub_piece_infos_.size();
            ar & priority_;
        }

        LiveRequestSubPiecePacket()
        {
        }

        LiveRequestSubPiecePacket(
            boost::uint32_t transaction_id, const std::string & rid,
            const std::vector<LiveSubPieceInfo>& sub_piece_infos, boost::uint16_t priority, 
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            sub_piece_infos_ = sub_piece_infos;
            priority_ = priority;
            end_point_ = endpoint;
        }

        virtual boost::uint32_t length() const
        {
            boost::uint32_t len = CommonPeerPacket::length() + sizeof(request_sub_piece_count_) +
                sub_piece_infos_.size() * LiveSubPieceInfo::length() + sizeof(priority_);

            return len;
        }

        boost::uint8_t request_sub_piece_count_;
        std::vector<LiveSubPieceInfo> sub_piece_infos_;
        boost::uint16_t priority_;
        static const boost::uint16_t DEFAULT_LIVE_PRIORITY = 9;
    };

    /**
    *@brief  LiveSubPiece 包
    */
    struct LiveSubPiecePacket
        : CommonPeerPacketT<0xC3>
    {
        LiveSubPiecePacket()
        {
            sub_piece_content_.reset(new LiveSubPieceContent);
            assert(sub_piece_content_);
        }

        LiveSubPiecePacket(
            boost::uint32_t transaction_id,
            const std::string & rid,
            const LiveSubPieceInfo & sub_piece_info,
            boost::uint16_t sub_piece_length,
            const LiveSubPieceBuffer & sub_piece_content,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            rid_ = rid;
            sub_piece_info_ = sub_piece_info;
            sub_piece_length_ = sub_piece_length;
            sub_piece_content_ = sub_piece_content.GetSubPieceBuffer();
            end_point_ = endpoint;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            if (!sub_piece_content_ || !*sub_piece_content_)
            {
                ar.fail();
                return;
            }
            CommonPeerPacket::serialize(ar);
            ar & sub_piece_info_;
            ar & sub_piece_length_ ;
            ar & framework::container::make_array(sub_piece_content_->get_buffer(), sub_piece_length_);
        }

        virtual boost::uint32_t length() const
        {
            return CommonPeerPacket::length() + sizeof(sub_piece_info_) + sizeof( sub_piece_length_ )
                + sub_piece_length_;
        }


        LiveSubPieceContent::pointer sub_piece_content_;
        boost::uint16_t sub_piece_length_;
        LiveSubPieceInfo sub_piece_info_;
    };

    struct PeerInfo
    {
        PeerInfo()
        {
            peer_info_length_ = 0;
            download_connected_count_ = 0;
            upload_connected_count_ = 0;
            upload_speed_ = 0;
            max_upload_speed_ = 0;
            rest_playable_time_ = 0;
            lost_rate_ = 0;
            redundancy_rate_ = 0;
        }

        PeerInfo(boost::uint8_t download_connected_count,
            boost::uint8_t upload_connected_count,
            boost::uint32_t upload_speed,
            boost::uint32_t max_upload_speed,
            boost::uint32_t rest_playable_time,
            boost::uint8_t lost_rate,
            boost::uint8_t redundancy_rate)
        {
            download_connected_count_ = download_connected_count;
            upload_connected_count_ = upload_connected_count;
            upload_speed_ = upload_speed;
            max_upload_speed_ = max_upload_speed;
            rest_playable_time_ = rest_playable_time;
            lost_rate_ = lost_rate;
            redundancy_rate_ = redundancy_rate;

            peer_info_length_ = sizeof(download_connected_count_) + sizeof(upload_connected_count_) + 
                sizeof(upload_speed_) + sizeof(max_upload_speed_) + sizeof(rest_playable_time_) + sizeof(lost_rate_) +
                sizeof(redundancy_rate_);
        }

        boost::uint32_t peer_info_length_;
        boost::uint8_t download_connected_count_;
        boost::uint8_t upload_connected_count_;
        boost::uint32_t upload_speed_;
        boost::uint32_t max_upload_speed_;
        boost::uint32_t rest_playable_time_;
        boost::uint8_t lost_rate_;
        boost::uint8_t redundancy_rate_;
    };

    template <typename Archive>
    void serialize(Archive & ar, PeerInfo & t)
    {
        util::serialization::split_free(ar, t);
    }

    template <typename Archive>
    void load(Archive & ar, PeerInfo & t)
    {
        ar & t.peer_info_length_;

        boost::uint32_t peer_info_length = t.peer_info_length_;

        if (peer_info_length >= sizeof(t.download_connected_count_))
        {
            ar & t.download_connected_count_;
            peer_info_length -= sizeof(t.download_connected_count_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.upload_connected_count_))
        {
            ar & t.upload_connected_count_;
            peer_info_length -= sizeof(t.upload_connected_count_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.upload_speed_))
        {
            ar & t.upload_speed_;
            peer_info_length -= sizeof(t.upload_speed_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.max_upload_speed_))
        {
            ar & t.max_upload_speed_;
            peer_info_length -= sizeof(t.max_upload_speed_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.rest_playable_time_))
        {
            ar & t.rest_playable_time_;
            peer_info_length -= sizeof(t.rest_playable_time_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.lost_rate_))
        {
            ar & t.lost_rate_;
            peer_info_length -= sizeof(t.lost_rate_);
        }
        else
        {
            return;
        }

        if (peer_info_length >= sizeof(t.redundancy_rate_))
        {
            ar & t.redundancy_rate_;
            peer_info_length -= sizeof(t.redundancy_rate_);
        }
        else
        {
            return;
        }
    }


    template <typename Archive>
    void save(Archive & ar, PeerInfo const & t)
    {
        assert(t.peer_info_length_ == sizeof(t.download_connected_count_) + sizeof(t.upload_connected_count_) + 
            sizeof(t.upload_speed_) + sizeof(t.max_upload_speed_) + sizeof(t.rest_playable_time_) + sizeof(t.lost_rate_) +
            sizeof(t.redundancy_rate_));

        ar & t.peer_info_length_;
        ar & t.download_connected_count_;
        ar & t.upload_connected_count_;
        ar & t.upload_speed_;
        ar & t.max_upload_speed_;
        ar & t.rest_playable_time_;
        ar & t.lost_rate_;
        ar & t.redundancy_rate_;
    }

    /**
    *@brief  PeerInfoPacket 包
    */
    struct PeerInfoPacket
        : PacketT<0xC4>
        , Packet
    {
        PeerInfoPacket()
        {
        }

        PeerInfoPacket(
            boost::uint32_t transaction_id,
            boost::uint16_t protocol_version,
            const PeerInfo & peer_info,
            const boost::asio::ip::udp::endpoint & endpoint)
        {
            transaction_id_ = transaction_id;
            protocol_version_ = protocol_version;
            peer_info_ = peer_info;
            end_point_ = endpoint;
        }

        PeerInfoPacket(
            boost::uint32_t transaction_id,
            boost::uint16_t protocol_version,
            const PeerInfo & peer_info)
        {
            transaction_id_ = transaction_id;
            protocol_version_ = protocol_version;
            peer_info_ = peer_info;
        }

        template <typename Archive>
        void serialize(Archive & ar)
        {
            Packet::serialize(ar);
            ar & peer_info_;
        }

        virtual boost::uint32_t length() const
        {
            return Packet::length() + sizeof(PeerInfo);
        }

        PeerInfo peer_info_;
    };
}

#endif // LIVE_PEER_PACKET_H_
