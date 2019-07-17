#ifndef _H264_RECORD_H_
#define _H264_RECORD_H_

#include <limits.h>
#include <stdint.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
}

#include "byte_stream.hpp"

class H264Record {
public:
    H264Record(void);
    ~H264Record(void);

public:
    int init(const char* filename);
    int uninit();

    int write_data(uint8_t* data, int len);
    void write_video_frame(uint8_t* p, int len);

    int create(uint8_t* p, int len);
    void finish();

private:
    int fill_extradata(uint8_t* data, int len);
    int add_video_stream(uint8_t* data, int len);

private:
    char recordFileName_[PATH_MAX];
    AVFormatContext* formatCtx_;

    int videoStreamdIndex_; // video stream index
    int frameCount_;

    ByteStream accessUnitBuf_;
    ByteStream extraData_;
};

#endif
