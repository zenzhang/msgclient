#ifndef _STORAGE_BUFFER_H_
#define _STORAGE_BUFFER_H_

#include "SubPieceContent.h"

namespace protocol
{
    struct LiveSubPieceBuffer
    {
    private:
        LiveSubPieceContent::pointer data_;
        boost::uint32_t length_;
        boost::uint32_t offset_;

    public:
        LiveSubPieceBuffer()
            : length_(0)
            , offset_(0)
        {
        }

        LiveSubPieceBuffer(
            LiveSubPieceBuffer const & buffer)
        {
            if (this != &buffer)
            {
                data_ = buffer.data_;
                length_ = buffer.length_;
                offset_ = buffer.offset_;
            }
        }

        LiveSubPieceBuffer & operator = (
            LiveSubPieceBuffer const & buffer)
        {
            if (this != &buffer)
            {
                data_ = buffer.data_;
                length_ = buffer.length_;
                offset_ = buffer.offset_;
            }
            return *this;
        }

        LiveSubPieceBuffer(
            LiveSubPieceContent * data,
            boost::uint32_t length = LiveSubPieceContent::sub_piece_size)
            : data_(data)
            , length_(length)
            , offset_(0)
        {
        }

        LiveSubPieceBuffer(
            LiveSubPieceContent::pointer const & data,
            boost::uint32_t length = LiveSubPieceContent::sub_piece_size)
            : data_(data)
            , length_(length)
            , offset_(0)
        {
        }

        LiveSubPieceContent::pointer GetSubPieceBuffer() const
        {
            return data_;
        }

        boost::uint8_t * Data()
        {
            return *data_;
        }
/*
        boost::uint8_t * Data(
            uint32_t offset)
        {
            return (boost::uint8_t *)*data_ + offset;
        }
*/
        boost::uint8_t const * Data() const
        {
            return data_ ? (boost::uint8_t*)(*data_) : NULL;
        }
/*
        boost::uint8_t const * Data(
            uint32_t offset) const
        {
            return (boost::uint8_t const *)*data_ + offset;
        }
*/
        boost::uint32_t Length() const
        {
            return length_;
        }

        boost::uint32_t Offset() const
        {
            return offset_;
        }

        void Data(
            LiveSubPieceContent::pointer const & data)
        {
            data_ = data;
        }

        void Length(
            boost::uint32_t length)
        {
            length_ = length;
        }

        bool IsValid(boost::uint32_t subpiece_max_length) const
        {
            return Data() && length_ > 0 && length_ <= subpiece_max_length;
        }
/*
        void Offset(
            uint32_t offset)
        {
            offset_ = offset;
        }

        SubPieceBuffer SubBuffer(
            uint32_t offset) const
        {
            SubPieceBuffer buffer;
            if (offset < length_)
            {
                buffer.offset_ = offset_ + offset;
                buffer.length_ = length_ - offset;
                buffer.data_ = data_;
            }
            return buffer;
        }

        SubPieceBuffer SubBuffer(
            uint32_t offset,
            uint32_t length) const
        {
            SubPieceBuffer buffer;
            if (offset + length <= length_)
            {
                buffer.offset_ = offset_ + offset;
                buffer.length_ = length;
                buffer.data_ = data_;
            }
            return buffer;
        }

        void Clear(boost::uint8_t padding = 0)
        {
            if (length_ > 0)
            {
                memset(Data(), padding, length_);
            }
        }
*/
        bool operator !() const
        {
            return !data_;
        }

        operator bool() const
        {
            return data_ != NULL;
        }

        bool operator == (const LiveSubPieceBuffer & buffer) const
        {
            return data_.get() == buffer.data_.get() && length_ == buffer.length_ && offset_ == buffer.offset_;
        }

        friend bool operator <(
            const LiveSubPieceBuffer & b1,
            const LiveSubPieceBuffer & b2)
        {
            return b1.data_.get() < b2.data_.get();
        }
    };
}
#endif  // _STORAGE_BUFFER_H_
