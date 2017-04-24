#ifndef _STORAGE_SUB_PIECE_BUFFER_H_
#define _STORAGE_SUB_PIECE_BUFFER_H_

#include <boost/intrusive_ptr.hpp>

namespace protocol
{
    class LiveSubPieceContent;

    template <typename T>
    T buffer_static_cast(LiveSubPieceContent & buf);

    template <typename T>
    T buffer_static_cast(LiveSubPieceContent const & buf);

    class LiveSubPieceContent
    {
    public:
        typedef boost::intrusive_ptr<LiveSubPieceContent> pointer;

        LiveSubPieceContent()
            : nref_(0)
        {
        }

        ~LiveSubPieceContent()
        {
            assert(nref_ == 0);
        }

    public:
        operator boost::uint8_t *()
        {
            return buffer_;
        }

        operator boost::uint8_t const *() const
        {
            return buffer_;
        }

        template <typename T>
        friend T buffer_static_cast(
            LiveSubPieceContent & buf)
        {
            return static_cast<T>(buf.buffer_);
        }

        template <typename T>
        friend T buffer_static_cast(
            LiveSubPieceContent const & buf)
        {
            return static_cast<T>(static_cast<boost::uint8_t const *>(buf.buffer_));
        }

        boost::uint8_t *get_buffer()
        {
            return buffer_;
        }

    private:
        // non copyable
        LiveSubPieceContent(
            LiveSubPieceContent const &);

        LiveSubPieceContent & operator = (
            LiveSubPieceContent const &);

    private:
        friend void intrusive_ptr_add_ref(
            LiveSubPieceContent * p)
        {
            ++p->nref_;
        }

        friend void intrusive_ptr_release(
            LiveSubPieceContent * p)
        {
            if (--p->nref_ == 0) {
                delete p;
            }
        }

    public:
        static const boost::uint32_t sub_piece_size = LIVE_SUB_PIECE_SIZE;

    private:
        boost::uint8_t nref_;
        boost::uint8_t resv_[2];
        boost::uint8_t buffer_[LIVE_SUB_PIECE_SIZE];
        boost::uint8_t resv2_;
    };
}

#endif  // _STORAGE_SUB_PIECE_BUFFER_H_
