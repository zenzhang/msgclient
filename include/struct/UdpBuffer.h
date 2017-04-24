#ifndef _PEER_NETWORK_UDP_BUFFER_H_
#define _PEER_NETWORK_UDP_BUFFER_H_

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>

#include <struct/SubPieceBuffer.h>

namespace boost
{
    namespace asio
    {
        class mutable_buffers_3
        {
        public:
            typedef mutable_buffer value_type;

            typedef const mutable_buffer* const_iterator;

            mutable_buffers_3(
                mutable_buffer const & buffer0,
                mutable_buffer const & buffer1,
                mutable_buffer const & buffer2)
            {
                buffers[0] = buffer0;
                buffers[1] = buffer1;
                buffers[2] = buffer2;
            }

            mutable_buffers_3(
                mutable_buffer const & buffer0,
                mutable_buffer const & buffer1)
            {
                buffers[0] = buffer0;
                buffers[1] = buffer1;
            }

            mutable_buffers_3(
                mutable_buffer const & buffer0)
            {
                buffers[0] = buffer0;
            }

            const_iterator begin() const
            {
                return buffers;
            }

            const_iterator end() const
            {
                return buffers + 3;
            }

        private:
            mutable_buffer buffers[3];
        };

        class const_buffers_3
        {
        public:
            typedef const_buffer value_type;

            typedef const const_buffer* const_iterator;

            const_buffers_3(
                const_buffer const & buffer0)
            {
                buffers[0] = buffer0;
            }

            const_buffers_3(
                const_buffer const & buffer0,
                const_buffer const & buffer1)
            {
                buffers[0] = buffer0;
                buffers[1] = buffer1;
            }

            const_buffers_3(
                const_buffer const & buffer0,
                const_buffer const & buffer1,
                const_buffer const & buffer2)
            {
                buffers[0] = buffer0;
                buffers[1] = buffer1;
                buffers[2] = buffer2;
            }

            const_iterator begin() const
            {
                return buffers;
            }

            const_iterator end() const
            {
                if (boost::asio::buffer_cast<void const *>(buffers[2]))
                    return buffers + 3;
                else if (boost::asio::buffer_cast<void const *>(buffers[1]))
                    return buffers + 2;
                else
                    return buffers + 1;
            }

        private:
            const_buffer buffers[3];
        };

    }
}

namespace protocol
{
    typedef std::basic_istream<char> IUdpBufferStream;
    typedef std::basic_ostream<char> OUdpBufferStream;

    class UdpBuffer
        : public std::basic_streambuf<char>
    {
    public:
        UdpBuffer()
            : sub_piece_buffer_(new LiveSubPieceContent)
        {
            assert(sub_piece_buffer_);
            setp((char*)head_buffer_, (char*)head_buffer_ + head_size);
            setg((char*)head_buffer_, (char*)head_buffer_, (char*)head_buffer_);
        }

    public:
        boost::asio::ip::udp::endpoint & end_point()
        {
            return endpoint_;
        }

        boost::asio::ip::udp::endpoint const & end_point() const
        {
            return endpoint_;
        }

        void end_point(
            boost::asio::ip::udp::endpoint const & ep)
        {
            endpoint_ = ep;
        }

    public:
        typedef void (*unspecified_bool_type)();
        static void unspecified_bool_true() {}
        operator unspecified_bool_type() const
        {
            return (sub_piece_buffer_) ? unspecified_bool_true : 0;
        }

    public:
        boost::asio::mutable_buffers_3 prepare()
        {
            assert(pptr() == (char*)head_buffer_);
            if (pbase() == (char*)head_buffer_) {
                return boost::asio::mutable_buffers_3(
                    boost::asio::mutable_buffer(pptr(), epptr() - pptr()),
                    boost::asio::mutable_buffer(sub_piece_buffer(), sub_piece_size()),
                    boost::asio::mutable_buffer(tail_buffer_, tail_size));
            }
            else if (pbase() == (char*)sub_piece_buffer()) {
                return boost::asio::mutable_buffers_3(
                    boost::asio::mutable_buffer(pptr(), epptr() - pptr()),
                    boost::asio::mutable_buffer(tail_buffer_, tail_size));
            } else {
                return boost::asio::mutable_buffers_3(
                    boost::asio::mutable_buffer(pptr(), epptr() - pptr()));
            }
        }

        void commit(
            boost::uint32_t size)
        {
            if ((boost::uint32_t)(epptr() - pptr()) >= size) {
                pbump(static_cast<int>(size));
            } else {
                size -= epptr() - pptr();
                if (pbase() == (char*)head_buffer_) {
                    if (size <= sub_piece_size()) {
                        setp((char*)sub_piece_buffer(), (char*)sub_piece_buffer() + sub_piece_size());
                        pbump(static_cast<int>(size));
                        return;
                    } else {
                        size -= sub_piece_size();
                    }
                }
                assert(size <= tail_size);
                setp((char*)tail_buffer_, (char*)tail_buffer_ + tail_size);
                pbump(static_cast<int>(size));
            }
        }

        boost::asio::const_buffers_3 data() const
        {
            if (eback() == (char*)head_buffer_) {
                if (pbase() == (char*)head_buffer_) {
                    return boost::asio::const_buffers_3(
                        boost::asio::const_buffer(gptr(), pptr() - gptr()));
                } else if (pbase() == (char*)sub_piece_buffer()) {
                    return boost::asio::const_buffers_3(
                        boost::asio::const_buffer(gptr(), (char*)head_buffer_ + head_size - gptr()),
                        boost::asio::const_buffer(sub_piece_buffer(), pptr() - (char*)sub_piece_buffer()));
                } else {
                    return boost::asio::const_buffers_3(
                        boost::asio::const_buffer(gptr(), (char*)head_buffer_ + head_size - gptr()),
                        boost::asio::const_buffer(sub_piece_buffer(), sub_piece_size()),
                        boost::asio::const_buffer(tail_buffer_, pptr() - (char*)tail_buffer_));
                }
            }
            else if (eback() == (char*)sub_piece_buffer()) {
                if (pbase() == (char*)sub_piece_buffer()) {
                    return boost::asio::const_buffers_3(
                        boost::asio::const_buffer(gptr(), pptr() - gptr()));
                } else {
                    return boost::asio::const_buffers_3(
                        boost::asio::const_buffer(gptr(), (char*)sub_piece_buffer() + sub_piece_size() - gptr()),
                        boost::asio::const_buffer(tail_buffer_, pptr() - (char*)tail_buffer_));
                }
            } else {
                return boost::asio::const_buffers_3(
                    boost::asio::const_buffer(gptr(), pptr() - gptr()));
            }
        }

        boost::uint32_t size() const
        {
            boost::uint32_t n = 0;
            if (pbase() == (char*)head_buffer_) {
                n = (pptr() - (char*)head_buffer_);
            }
            else if (pbase() == (char*)sub_piece_buffer()) {
                n = head_size + (pptr() - (char*)sub_piece_buffer());
            } else {
                n = head_size + sub_piece_size() + (pptr() - (char*)tail_buffer_);
            }
            if (eback() == (char*)head_buffer_) {
                n -= gptr() - (char*)head_buffer_;
            }
            else if (eback() == (char*)sub_piece_buffer()) {
                n -= head_size + (gptr() - (char*)sub_piece_buffer());
            } else {
                n -= head_size + sub_piece_size() + (gptr() - (char*)tail_buffer_);
            }
            return n;
        }

        void consume(
            boost::uint32_t size)
        {
            if ((boost::uint32_t)(egptr() - gptr()) >= size) {
                gbump(static_cast<int>(size));
            } else {
                size -= egptr() - gptr();
                do {
                    if (eback() == (char*)head_buffer_) {
                        if (egptr() + size <= (char*)head_buffer_ + head_size) {
                            setg((char*)head_buffer_, gptr() + size, (char*)head_buffer_ + head_size);
                            break;
                        } else {
                            size -= (char*)head_buffer_ + head_size - egptr();
                        }
                        if (size <= sub_piece_size()) {
                            setg((char*)sub_piece_buffer(), (char*)sub_piece_buffer() + size, (char*)sub_piece_buffer() + sub_piece_size());
                            break;
                        } else {
                            size -= sub_piece_size();
                        }
                        assert(size <= tail_size);
                        setg((char*)tail_buffer_, (char*)tail_buffer_ + size, (char*)tail_buffer_ + tail_size);
                    }
                    else if (eback() == (char*)sub_piece_buffer()) {
                        if (egptr() + size <= (char*)sub_piece_buffer() + sub_piece_size()) {
                            setg((char*)sub_piece_buffer(), gptr() + size, (char*)sub_piece_buffer() + sub_piece_size());
                            break;
                        } else {
                            size -= (char*)head_buffer_ + head_size - egptr();
                        }
                        assert(size <= tail_size);
                        setg((char*)tail_buffer_, (char*)tail_buffer_ + size, (char*)tail_buffer_ + tail_size);
                    } else {
                        assert(size <= tail_size);
                        setg((char*)tail_buffer_, (char*)tail_buffer_ + size, (char*)tail_buffer_ + tail_size);
                    }
                    break;
                } while (1);
                if (eback() == pbase()) {
                    assert(gptr() <= pptr());
                    setg(eback(), gptr(), pptr());
                }
            }
        }

        void reset()
        {
            setp((char*)head_buffer_, (char*)head_buffer_ + head_size);
            setg((char*)head_buffer_, (char*)head_buffer_, (char*)head_buffer_);
        }

        boost::uint8_t * GetHeadBuffer()
        {
            return head_buffer_;
        }

    protected:
        boost::uint8_t * sub_piece_buffer()
        {
            if (sub_piece_buffer_ && *sub_piece_buffer_)
                return buffer_static_cast<boost::uint8_t *>(*sub_piece_buffer_) + 1;
            else
                return NULL;
        }

        boost::uint8_t const * sub_piece_buffer() const
        {
            if (sub_piece_buffer_ && *sub_piece_buffer_)
                return buffer_static_cast<boost::uint8_t const *>(*sub_piece_buffer_) + 1;
            return NULL;
        }

        boost::uint32_t sub_piece_size() const
        {
            return LiveSubPieceContent::sub_piece_size;
        }

    protected:
        int_type underflow()
        {
            if (gptr() != pptr())
            {
                if (eback() == (char*)head_buffer_) {
                    if (egptr() == (char*)head_buffer_ + head_size)
                        setg((char*)sub_piece_buffer(), (char*)sub_piece_buffer(), (char*)sub_piece_buffer() + sub_piece_size());
                    else
                        setg((char*)head_buffer_, (char*)head_buffer_, (char*)head_buffer_ + head_size);
                }
                else if (eback() == (char*)sub_piece_buffer()) {
                    if (egptr() == (char*)sub_piece_buffer() + sub_piece_size())
                        setg((char*)tail_buffer_, (char*)tail_buffer_, (char*)tail_buffer_ + tail_size);
                    else
                        setg((char*)sub_piece_buffer(), (char*)sub_piece_buffer(), (char*)sub_piece_buffer() + sub_piece_size());
                } else {
                    setg((char*)tail_buffer_, (char*)tail_buffer_, (char*)tail_buffer_ + tail_size);
                }
                if (eback() == pbase()) {
                    setg(eback(), gptr(), pptr());
                }
                return traits_type::to_int_type(*gptr());
            }
            else
            {
                return traits_type::eof();
            }
        }

        int_type overflow(int_type c)
        {
            if (!traits_type::eq_int_type(c, traits_type::eof()))
            {
                assert(pbase() != (char*)tail_buffer_);
                if (pbase() == (char*)head_buffer_)
                    setp((char*)sub_piece_buffer(), (char*)sub_piece_buffer() + sub_piece_size());
                else
                    setp((char*)tail_buffer_, (char*)tail_buffer_ + tail_size);
                *pptr() = traits_type::to_char_type(c);
                pbump(1);
                return c;
            }
            return traits_type::not_eof(c);
        }

    private:
        // non copyable
        UdpBuffer(
            UdpBuffer const &);

        UdpBuffer & operator = (
            UdpBuffer const &);

    private:
        static const boost::uint32_t head_size = 4 + 1 + 4 + 4 + 16 + 16 + 4 + 2 + 1;
        static const boost::uint32_t tail_size = 1024;

    public:
        boost::asio::ip::udp::endpoint endpoint_;
        boost::uint8_t head_buffer_[head_size];
        LiveSubPieceContent::pointer sub_piece_buffer_;
        boost::uint8_t tail_buffer_[tail_size];
    };

}

#endif  // _PEER_NETWORK_UDP_BUFFER_H_
