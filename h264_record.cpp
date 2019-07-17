
#include "h264_record.hpp"
#include "h264_reader.hpp"
#include "sps_parser.hpp"
#include "utils.hpp"

#include <stdio.h>
#include <string.h>

H264Record::H264Record(void)
    : accessUnitBuf_(1000 * 1000)
    , extraData_(100)
{
    videoStreamdIndex_ = -1;
    frameCount_ = -1;
    formatCtx_ = NULL;
}

H264Record::~H264Record(void)
{
}

int H264Record::init(const char* filename)
{
    if (filename == NULL)
        return -1;

    strcpy(recordFileName_, filename);
    return 0;
}

int H264Record::uninit()
{
    finish();
    return 0;
}

int H264Record::write_data(uint8_t* data, int len)
{

    if (data == NULL || len < 0)
        return -1;

    // find sps
    if (!formatCtx_ && fill_extradata(data, len) == 0) {
        create(extraData_.bytes(), extraData_.len());
    }

    H264Reader reader(data, len);
    uint8_t* nalus;
    int nalus_len;

    // find access unit
    while (reader.seek_next_access_unit()) {
        reader.read_until_next_acu(nalus, nalus_len);

        accessUnitBuf_.write(nalus, nalus_len);

        write_video_frame(accessUnitBuf_.bytes(), accessUnitBuf_.len());
        accessUnitBuf_.reset();
    }

    reader.unread(data, len);
    accessUnitBuf_.write(data, len);

    return 0;
}

void H264Record::write_video_frame(uint8_t* p, int len)
{
    // fprintf(stdout, "frame %d: ", len);
    // for(int i = 0; i < 30 && i < len; i++){
    //     fprintf(stdout, "%d ", p[i]);
    // }
    // fprintf(stdout, "\n");

    if (!formatCtx_ || videoStreamdIndex_ < 0)
        return;

    frameCount_++;
    // fprintf(stderr, "frame count=%d\n", frameCount_);

    // AVStream *pst = formatCtx_->streams[ videoStreamdIndex_ ];
    AVStream* vSt = formatCtx_->streams[videoStreamdIndex_];

    // Init packet
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.stream_index = vSt->index;

    pkt.data = (uint8_t*)av_malloc(len);
    pkt.size = len;

    memcpy(pkt.data, p, len);

    // Wait for key frame_count
    pkt.dts = frameCount_ * 90000 / 25;
    pkt.pts = pkt.dts;
    pkt.duration = 90000 / 25;

    // fprintf(stdout, "frame dts=%ld\n", pkt.dts);

    // av_pkt_dump2(stdout, &pkt, 1, formatCtx_->streams[videoStreamdIndex_]);
    int ret = av_interleaved_write_frame(formatCtx_, &pkt);
    if (ret != 0) {
        fprintf(stderr, "write interleaved frame failed\n");
    }
}

int H264Record::create(uint8_t* p, int len)
{
    if (get_nal_type(p, len) != 0x07) {
        fprintf(stderr, "should create with sps nalu\n");
        return -1;
    }

    av_register_all();
    avcodec_register_all();

    // open output context
    if (avformat_alloc_output_context2(
            &formatCtx_, NULL, "mp4", recordFileName_)
        < 0) {
        fprintf(stderr, "alloc output context failed.\n");
        return -1;
    }

    auto ofmt = formatCtx_->oformat;
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&formatCtx_->pb, recordFileName_, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open output file '%s'", recordFileName_);
            goto FAIL;
        }
    }

    // assign new stream
    if (add_video_stream(p, len) < 0) {
        goto FAIL;
    }

    // write header
    if (avformat_write_header(formatCtx_, NULL) < 0) {
        fprintf(stderr, "write header failed\n");
        goto FAIL;
    }

    return 0;

FAIL:
    avformat_free_context(formatCtx_);
    formatCtx_ = NULL;

    return -1;
}

void H264Record::finish()
{
    av_interleaved_write_frame(formatCtx_, NULL);
    av_write_trailer(formatCtx_);
    avformat_free_context(formatCtx_);

    videoStreamdIndex_ = -1;
    frameCount_ = -1;
    formatCtx_ = NULL;
}

// the AVStream->codecpar->extradata must conform to the following tow step,
// otherwise the api will not tranform the annexb start code prefix to the mp4 prefix.
int H264Record::fill_extradata(uint8_t* data, int len)
{
    uint8_t* nalu;
    int nalu_len;
    H264Reader reader(data, len);
    uint8_t start_code_prefix[2][4] = { { 0, 0, 1 }, { 0, 0, 0, 1 } };

    // fprintf(stdout, "frame %d: ", len);
    // for (int i = 0; i < 100 && i < len; i++) {
    //     fprintf(stdout, "%0x ", data[i]);
    // }
    // fprintf(stdout, "\n");

    if (extraData_.len() == 0) { // first step: fill sps with prefix  00 00 01
        if (!reader.seek_sps()) {
            return -1;
        }

        if (reader.read_nalu(nalu, nalu_len) < 0)
            return -1;

        // fprintf(stdout, "sps %d: ", nalu_len);
        // for (int i = 0; i < 30 && i < nalu_len; i++) {
        //     fprintf(stdout, "%0x ", nalu[i]);
        // }
        // fprintf(stdout, "\n");

        trim_annexb_prefix(nalu, nalu_len);

        extraData_.write(start_code_prefix[0], 3);
        extraData_.write(nalu, nalu_len);
    }

    if (extraData_.len() > 0) { // second step: fill any nalu with prefix  00 00 00 01, the second nalu can be incomplete
        if (reader.read_nalu(nalu, nalu_len) < 0)
            return -1;

        // fprintf(stdout, "nalu 2 %d: ", nalu_len);
        // for (int i = 0; i < 30 && i < nalu_len; i++) {
        //     fprintf(stdout, "%0x ", nalu[i]);
        // }
        // fprintf(stdout, "\n");

        trim_annexb_prefix(nalu, nalu_len);

        extraData_.write(start_code_prefix[1], 4);
        extraData_.write(nalu, nalu_len);

        return 0;
    }

    return -1;
}

int H264Record::add_video_stream(uint8_t* data, int len)
{
    AVStream* stream = avformat_new_stream(formatCtx_, NULL);
    if (!stream) {
        fprintf(stderr, "open video avstream failed\n");
        return -1;
    }

    stream->id = 0;

    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->codec_id = AV_CODEC_ID_H264;

    stream->codecpar->extradata_size = len;
    stream->codecpar->extradata = (uint8_t*)av_malloc(len);
    memcpy(stream->codecpar->extradata, data, len);

    // av_hex_dump(stdout, stream->codecpar->extradata, stream->codecpar->extradata_size);

    SeqParameterSet* sps = SeqParameterSet::parse(data, len);

    stream->codecpar->width = sps->width;
    stream->codecpar->height = sps->height;

    stream->codecpar->profile = sps->profile_idc;
    stream->codecpar->level = sps->level_idc;

    stream->codecpar->codec_tag = 0;
    stream->codecpar->codec_id = AV_CODEC_ID_H264;

    printf("sps width=%d, height=%d\n", sps->width, sps->height);

    stream->time_base.num = 1;
    stream->time_base.den = 90000;

    videoStreamdIndex_ = stream->index;

    return 0;
}
