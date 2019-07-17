#include "h264_reader.hpp"

#include <stddef.h>

H264Reader::H264Reader()
    : buf_(NULL)
    , size_(-1)
{
}

H264Reader::H264Reader(uint8_t* data, int len)
{
    buf_ = data;
    size_ = len;

    pos = buf_;
    pend = buf_ + size_;
}

H264Reader::~H264Reader() {}

int H264Reader::init(uint8_t* data, int len)
{
    if (data == NULL || len <= 0)
        return -1;

    buf_ = data;
    size_ = len;

    pos = buf_;
    pend = buf_ + size_;

    return 0;
}

int H264Reader::read_nalu(uint8_t*& buf, int& count)
{
    uint32_t prefix;
    uint8_t* last_pos = pos;

    if (pos >= (pend - 3)) {
        return -1;
    }

    // find prefix
    for (pos += 3; pos <= (pend - 3); pos++) {
        prefix = *((uint32_t*)pos);

        if ((prefix & 0x00FFFFFF) == 0x010000) {
            break;
        }

        if (prefix == 0x01000000) {
            break;
        }
    }

    if (pos >= (pend - 3)) {
        pos = pend;
    }

    buf = last_pos;
    count = pos - last_pos;

    return count;
}

bool H264Reader::seek_sps()
{
    uint32_t prefix;

    for (uint8_t* sps_pos = buf_; sps_pos <= (pend - 3); sps_pos++) {
        prefix = *((uint32_t*)sps_pos);

        if ((prefix & 0x1FFFFFFF) == 0x07010000) {
            if (sps_pos > buf_ && *(sps_pos - 1) == 0) {
                pos = sps_pos - 1;

            } else {
                pos = sps_pos;
            }

            return true;
        }
    }

    return false;
}

bool H264Reader::seek_next_access_unit()
{
    uint32_t prefix;
    uint8_t* au_pos;

    for (au_pos = pos; au_pos <= (pend - 3); au_pos++) {
        prefix = *((uint32_t*)au_pos);

        if ((prefix & 0x1FFFFFFF) == 0x09010000) {
            return true;
        }
    }

    return false;
}

int H264Reader::read_until_next_acu(uint8_t*& buf, int& count)
{
    uint32_t prefix;
    uint8_t* last_pos = pos;

    if (pos >= (pend - 3)) {
        return -1;
    }

    // find prefix
    for (; pos <= (pend - 3); pos++) {
        prefix = *((uint32_t*)pos);

        if ((prefix & 0x1FFFFFFF) == 0x09010000) {
            if (pos > buf_ && *(pos - 1) == 0) {
                pos -= 1;
            }

            break;
        }
    }

    if (pos >= (pend - 3)) {
        pos = pend;
    }

    buf = last_pos;
    count = pos - last_pos;

    // void repeat
    if (pos < pend - 3) {
        pos += 3;
    }

    return count;
}

int H264Reader::unread(uint8_t*& data, int& len)
{
    data = pos;
    len = pend - pos;

    return 0;
}
