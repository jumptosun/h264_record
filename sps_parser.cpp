#include "sps_parser.hpp"

#include <stdlib.h>

#include "bit_stream.hpp"
#include "utils.hpp"

SeqParameterSet* SeqParameterSet::parse(uint8_t* data, int len)
{
    uint8_t* buf = data;
    int length = len;

    if (len < 5) {
        return NULL;
    }

    // trim annexb prefix
    trim_annexb_prefix(buf, length);

    // determine the nal type
    int nalu_type = buf[0] & 0x1F;

    if (nalu_type != 7)
        return NULL;

    // start parse
    SeqParameterSet* sps = new SeqParameterSet();

    BitStream stream;
    stream.init(buf + 1, uint32_t(length - 1));

    sps->profile_idc = stream.get_bits(8);

    /* constraint_set0_flag = br_get_bit(br);    */
    /* constraint_set1_flag = br_get_bit(br);    */
    /* constraint_set2_flag = br_get_bit(br);    */
    /* constraint_set3_flag = br_get_bit(br);    */
    /* constraint_set4_flag = br_get_bit(br);    */
    /* constraint_set5_flag = br_get_bit(br);    */
    /* reserved             = br_get_bits(br,2); */
    stream.skip_bits(8);

    sps->level_idc = stream.get_bits(8);

    sps->seq_parameter_set_id = stream.get_ue_golomb();

    if (sps->profile_idc == 100 || sps->profile_idc == 110 || sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc == 44 || sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == 118 || sps->profile_idc == 128) {
        sps->chroma_format_idc = stream.get_ue_golomb();
        if (sps->chroma_format_idc == 3) {
            stream.skip_bits(1); /* separate_colour_plane_flag */
        }

        stream.skip_golomb(); /* bit_depth_luma - 8             */
        stream.skip_golomb(); /* bit_depth_chroma - 8           */
        stream.skip_bits(1); /* transform_bypass               */

        if (stream.get_bit()) { /* seq_scaling_matrix_present     */

            int count = (sps->chroma_format_idc != 3) ? 8 : 12;

            for (int i = 0; i < count; i++) {
                if (stream.get_bit()) { /* seq_scaling_list_present    */

                    int last = 8, next = 8, size = (i < 6) ? 16 : 64;

                    for (int j = 0; j < size; j++) {
                        if (next)
                            next = (last + stream.get_se_golomb()) & 0xff;
                        last = next ? next : last;
                    }
                }
            }
        }
    }

    stream.skip_golomb(); /* log2_max_frame_num - 4 */
    sps->pic_order_cnt_type = stream.get_ue_golomb();

    if (sps->pic_order_cnt_type == 0) {
        stream.skip_golomb(); /* log2_max_poc_lsb - 4 */

    } else if (sps->pic_order_cnt_type == 1) {
        stream.skip_bits(1); /* delta_pic_order_always_zero     */

        stream.skip_golomb(); /* offset_for_non_ref_pic          */
        stream.skip_golomb(); /* offset_for_top_to_bottom_field  */
        int j = stream.get_ue_golomb(); /* num_ref_frames_in_pic_order_cnt_cycle */

        for (int i = 0; i < j; i++)
            stream.skip_golomb(); /* offset_for_ref_frame[i]         */
    }

    stream.skip_golomb(); /* ref_frames                      */
    stream.skip_bits(1); /* gaps_in_frame_num_allowed       */

    sps->pic_width_in_mbs_minus1 = stream.get_ue_golomb();
    sps->pic_height_in_map_units_minus1 = stream.get_ue_golomb();
    sps->frame_mbs_only_flag = stream.get_bit();

    if (!sps->frame_mbs_only_flag)
        sps->mb_adaptive_frame_field_flag = stream.get_bit();

    stream.skip_bits(1); /* direct_8x8_inference_flag    */

    if (stream.get_bit()) {
        /* frame_cropping_flag */
        stream.skip_golomb();
        stream.skip_golomb();
        stream.skip_golomb();
        stream.skip_golomb();
    }

    /* VUI parameters */
    if (stream.get_bit()) {
        /* vui_parameters_present flag */

        if (stream.get_bit()) {
            /* aspect_ratio_info_present */
            uint32_t aspect_ratio_idc = stream.get_bits(8);

            if (aspect_ratio_idc == 255 /* Extended_SAR */) {
                stream.skip_bits(16); /* sar_width */
                stream.skip_bits(16); /* sar_height */
            }

            if (stream.get_bits(1)) { /* overscan_info_present_flag*/
                stream.skip_bits(1); // overscan_appropriate_flag
            }

            if (stream.get_bits(1)) { // video_signal_type_present_flag
                stream.skip_bits(3); // video_format
                stream.skip_bits(1); // video_full_range_flag

                if (stream.get_bits(1)) { // colour_description_present_flag
                    stream.skip_bits(8); //  colour_primaries
                    stream.skip_bits(8); //  transfer_uint8_tacteristics
                    stream.skip_bits(8); //  matrix_coefficients
                }
            }

            if (stream.get_bits(1)) { // chroma_loc_info_present_flag
                stream.skip_golomb(); // chroma_sample_loc_type_top_field
                stream.skip_golomb(); // chroma_sample_loc_type_bottom_field
            }

            if (stream.get_bits(1)) { // timing_info_present_flag
                sps->num_units_in_tick = stream.get_bits(32); // num_units_in_tick
                sps->time_scale = stream.get_bits(32); // time_scale
                sps->fixed_frame_rate_flag = stream.get_bit(); // fixed_frame_rate_flag

                if (sps->num_units_in_tick) {
                    sps->fps = sps->time_scale / sps->num_units_in_tick / (sps->fixed_frame_rate_flag + 1);
                }
            }
            // other hdr so on
        }
    }

    sps->width = (sps->pic_width_in_mbs_minus1 + 1) * 16;
    sps->height = (sps->pic_height_in_map_units_minus1 + 1) * 16 * (2 - sps->frame_mbs_only_flag);

    return sps;
}
