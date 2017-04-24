#ifndef _CHECKSUM_H_CK_20111122__
#define _CHECKSUM_H_CK_20111122__

template <typename ConstBufferSequence>
inline boost::uint32_t check_sum_new(
                                  ConstBufferSequence const & buffers)
{
    using namespace boost::asio;
    typedef typename ConstBufferSequence::const_iterator iterator;
    iterator iter = buffers.begin();
    iterator end = buffers.end();
    register boost::uint32_t crc = (boost::uint32_t)0x19870815;
    register boost::uint32_t v0 = 0;
    register boost::uint32_t v1 = 0;
    boost::uint32_t const * buf = NULL;
    boost::uint32_t size = 0;
    // 兼容老算法的BUG
    boost::uint8_t last_byte = 0;
    for (; iter != end; ++iter) {
        buf = buffer_cast<boost::uint32_t const *>(buffer(*iter));
        assert(size == 0);
        assert(((size_t)buf & (sizeof(boost::uint32_t) - 1)) == 0);  // 地址必须4字节对齐
        size = buffer_size(buffer(*iter));
        // 兼容老算法的BUG，最后8个字节被拆成8个字节独立计算了
        iterator iter1(iter);
        if (++iter1 == end) {
            --size;
            last_byte = ((boost::uint8_t const *)buf)[size];
        }
        while (size >= sizeof(boost::uint32_t) * 2) {
            v0 = *buf++;
            v1 = *buf++;
            crc ^= ((crc << 14) ^ (crc >> 6))
                ^ framework::system::BytesOrder::little_endian_to_host_long(v0 ^ v1);
            size -= sizeof(boost::uint32_t) * 2;
        }
    }
    if (size) {
        boost::uint8_t const * bytes = (boost::uint8_t const *)buf;
        while (size--) {
            crc ^= ((crc >> 13) ^ (*bytes++ & 0xFF) ^ (crc << 7));
        }
    }
    crc ^= ((crc >> 13) ^ (last_byte & 0xFF) ^ (crc << 7));
    return framework::system::BytesOrder::host_to_little_endian_long(crc);
}
#endif
