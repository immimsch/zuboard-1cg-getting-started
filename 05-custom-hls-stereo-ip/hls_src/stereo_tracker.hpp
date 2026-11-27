#ifndef STEREO_TRACKER_HPP
#define STEREO_TRACKER_HPP

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

// Resolution for a single eye (640x480); input stream packs Left (7:0) and Right (15:8)
constexpr int IMG_WIDTH  = 640;
constexpr int IMG_HEIGHT = 480;
constexpr int WIN_SIZE   = 5;
constexpr int MAX_DISP   = 32;

// 16-bit packed stereo input pixel (bits [7:0] = Left, bits [15:8] = Right)
typedef ap_axiu<16, 1, 1, 1> stereo_axis_t;
// 8-bit output disparity pixel (scaled by 8x for visualisation)
typedef ap_axiu<8, 1, 1, 1>  disp_axis_t;

void stereo_tracker_accel(
    hls::stream<stereo_axis_t> &in_stream,
    hls::stream<disp_axis_t>   &out_disp_stream,
    ap_uint<8>                  thresh,
    ap_uint<16>                &bbox_min_x,
    ap_uint<16>                &bbox_max_x,
    ap_uint<16>                &bbox_min_y,
    ap_uint<16>                &bbox_max_y,
    ap_uint<32>                &target_area,
    ap_uint<64>                &disp_sum
);

#endif // STEREO_TRACKER_HPP
