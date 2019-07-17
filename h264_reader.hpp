#ifndef _H264_READER_H_
#define _H264_READER_H_

#include <stdint.h>

struct H264Reader {
    uint8_t* buf_;
    int size_;

    uint8_t *pos, *pend;

    H264Reader();
    H264Reader(uint8_t* data, int len);
    ~H264Reader();

    int init(uint8_t* data, int len);

    int read_nalu(uint8_t*& buf, int& count);

    bool seek_sps();

    bool seek_next_access_unit();

    int read_until_next_acu(uint8_t*& buf, int& count);

    int unread(uint8_t*& data, int& len);
};

#endif