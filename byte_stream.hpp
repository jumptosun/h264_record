#ifndef _BYTE_STREAM_H_
#define _BYTE_STREAM_H_

#include <stdint.h>
#include <string.h>

class ByteStream {
public:
    ByteStream()
    {
        max_ = 1000;
        pos_ = 0;
        buf_ = new uint8_t[max_];
    }

    ByteStream(int max)
    {
        max_ = max;
        pos_ = 0;
        buf_ = new uint8_t[max_];
    }

    ~ByteStream()
    {
        if (buf_ != nullptr) {
            delete[] buf_;
            buf_ = nullptr;
        }
    }

    int write(uint8_t* data, int len)
    {
        int resize;
        uint8_t* rebuf;

        if (unuse() < len) {
            resize = (2 * max_) > (pos_ + len) ? 2 * max_ : pos_ + len;

            rebuf = new uint8_t[resize];
            memcpy(rebuf, buf_, pos_);

            delete[] buf_;
            buf_ = rebuf;
        }

        memcpy(buf_ + pos_, data, len);
        pos_ += len;

        return 0;
    }

    int reset()
    {
        pos_ = 0;
        return 0;
    }

    uint8_t* bytes()
    {
        return buf_;
    }

    int len()
    {
        return pos_;
    }

    int capacity()
    {
        return max_;
    }

    int unuse()
    {
        return max_ - pos_;
    }

private:
    uint8_t* buf_;
    int pos_;
    int max_;
};

#endif