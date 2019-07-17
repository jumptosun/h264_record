#ifndef _BIT_STREAM_H_
#define _BIT_STREAM_H_

#include <stdint.h>

class BitStream {
public:
    BitStream()
        : data_(NULL)
        , length_(-1)
        , index_(-1)
    {
    }
    ~BitStream() {}

    int init(uint8_t* data, uint32_t length)
    {
        if (data == NULL)
            return -1;

        data_ = data;
        length_ = length;
        index_ = 0;

        return 0;
    }

    int get_bit()
    {
        if (length_ == -1)
            return -1;

        if ((index_ >> 3) > length_)
            return -1;

        int ret = data_[index_ >> 3] >> (7 - (index_ & 7)) & 0x01;
        index_++;

        return ret;
    }

    int64_t get_bits(int n)
    {
        int64_t r = 0;

        if (length_ == -1)
            return -1;

        if ((index_ >> 3) > length_)
            return -1;

        if (n > 63)
            return -1;

        while (n--) {
            r |= get_bit() << n;
        }

        return r;
    }
    int skip_bits(int n)
    {
        if (length_ == -1)
            return -1;
        if ((index_ >> 3) > length_)
            return -1;

        index_ += n;
        return 0;
    }

    int64_t get_ue_golomb()
    {
        int m = 0;

        if (length_ == -1)
            return -1;
        if ((index_ >> 3) > length_)
            return -1;

        while (!get_bit() && m < 32)
            m++;

        return m ? ((1 << m) - 1) + get_bits(m) : 0;
    }

    int64_t get_se_golomb()
    {
        int64_t ret = 0;

        if (length_ == -1)
            return -1;
        if ((index_ >> 3) > length_)
            return -1;

        ret = get_ue_golomb();

        if (ret == -1)
            return ret;

        return ret & 0x01 ? -(ret >> 1) : ret >> 1;
    }

    int skip_golomb()
    {
        int m = 0;

        if (length_ == -1)
            return -1;
        if ((index_ >> 3) > length_)
            return -1;

        while (!get_bit() && m < 32)
            m++;

        skip_bits(m);

        return 0;
    }

private:
    uint8_t* data_;
    int64_t length_;
    int index_;
};

#endif