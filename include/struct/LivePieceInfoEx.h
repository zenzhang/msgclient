#ifndef _LIVE_PIECEINFOEX_H_
#define _LIVE_PIECEINFOEX_H_

namespace protocol
{
    struct LivePieceInfoEx
    {
        boost::uint32_t id_;
        boost::uint16_t subpiece_index_;

        LivePieceInfoEx()
            : id_(0), subpiece_index_(0)
        {
        }

        LivePieceInfoEx(boost::uint32_t id, boost::uint16_t subpiece_index)
            : id_(id), subpiece_index_(subpiece_index)
        {}

        LivePieceInfoEx(boost::uint32_t id)
            : id_(id), subpiece_index_(0)
        {}

        bool operator == (const LivePieceInfoEx & other)
        {
            return id_ == other.id_ && subpiece_index_ == other.subpiece_index_;
        }

        bool operator != (const LivePieceInfoEx & other)
        {
            return !operator==(other);
        }

        friend std::ostream& operator << (
            std::ostream& os, 
            const LivePieceInfoEx& n)
        {
            os << "(" << n.id_ << "|sp:" << n.subpiece_index_ << ")";
            return os;
        }

        friend bool operator < (const LivePieceInfoEx & piece1, const LivePieceInfoEx & piece2)
        {
            if (piece1.id_ < piece2.id_)
            {
                return true;
            }
            else if (piece1.id_ > piece2.id_)
            {
                return false;
            }
            else
            {
                return piece1.subpiece_index_ < piece2.subpiece_index_;
            }
        }
    };

}

#endif
