#ifndef _SPS_PARSER_H_
#define _SPS_PARSER_H_

#include <stdint.h>

class SeqParameterSet;

class SeqParameterSet {
public:
    SeqParameterSet()
        : width(-1)
        , height(-1)
        , fps(-1)
    {
    }
    ~SeqParameterSet() {}

    int profile_idc;
    int level_idc;
    int seq_parameter_set_id;

    int chroma_format_idc;
    int pic_order_cnt_type;

    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;

    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;

    int64_t num_units_in_tick;
    int64_t time_scale;
    int fixed_frame_rate_flag;

    int width;
    int height;
    int fps;

    static SeqParameterSet* parse(uint8_t* data, int len);
};

#endif