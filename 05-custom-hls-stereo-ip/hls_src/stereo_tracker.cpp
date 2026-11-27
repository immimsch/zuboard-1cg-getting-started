#include "stereo_tracker.hpp"

static inline ap_uint<8> abs_diff_u8(ap_uint<8> a, ap_uint<8> b) {
#pragma HLS INLINE
    return (a > b) ? (ap_uint<8>)(a - b) : (ap_uint<8>)(b - a);
}

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
) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_disp_stream
#pragma HLS INTERFACE s_axilite port=thresh      bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_min_x  bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_max_x  bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_min_y  bundle=control
#pragma HLS INTERFACE s_axilite port=bbox_max_y  bundle=control
#pragma HLS INTERFACE s_axilite port=target_area bundle=control
#pragma HLS INTERFACE s_axilite port=disp_sum    bundle=control
#pragma HLS INTERFACE s_axilite port=return      bundle=control

    // Line buffers for 5x5 stereo window
    static ap_uint<8> line_buf_L[WIN_SIZE - 1][IMG_WIDTH];
    static ap_uint<8> line_buf_R[WIN_SIZE - 1][IMG_WIDTH];
#pragma HLS ARRAY_PARTITION variable=line_buf_L complete dim=1
#pragma HLS ARRAY_PARTITION variable=line_buf_R complete dim=1

    // Left 5x5 window and Right 5x(5 + MAX_DISP) search strip
    ap_uint<8> win_L[WIN_SIZE][WIN_SIZE];
    ap_uint<8> win_R[WIN_SIZE][WIN_SIZE + MAX_DISP];
#pragma HLS ARRAY_PARTITION variable=win_L complete dim=0
#pragma HLS ARRAY_PARTITION variable=win_R complete dim=0

    // Hardware target tracking accumulators
    ap_uint<16> min_x = IMG_WIDTH;
    ap_uint<16> max_x = 0;
    ap_uint<16> min_y = IMG_HEIGHT;
    ap_uint<16> max_y = 0;
    ap_uint<32> area_cnt = 0;
    ap_uint<64> disp_acc = 0;

    row_loop:
    for (int y = 0; y < IMG_HEIGHT; y++) {
        col_loop:
        for (int x = 0; x < IMG_WIDTH; x++) {
#pragma HLS PIPELINE II=1

            stereo_axis_t pkt_in = in_stream.read();
            ap_uint<8> pix_L = pkt_in.data.range(7, 0);
            ap_uint<8> pix_R = pkt_in.data.range(15, 8);

            // Shift horizontal sliding windows
            for (int r = 0; r < WIN_SIZE; r++) {
#pragma HLS UNROLL
                for (int c = 0; c < WIN_SIZE - 1; c++) {
#pragma HLS UNROLL
                    win_L[r][c] = win_L[r][c + 1];
                }
                for (int c = 0; c < WIN_SIZE + MAX_DISP - 1; c++) {
#pragma HLS UNROLL
                    win_R[r][c] = win_R[r][c + 1];
                }
            }

            // Populate rightmost column from line buffers and current incoming pixel
            for (int r = 0; r < WIN_SIZE - 1; r++) {
#pragma HLS UNROLL
                ap_uint<8> val_L = line_buf_L[r][x];
                ap_uint<8> val_R = line_buf_R[r][x];
                win_L[r][WIN_SIZE - 1] = val_L;
                win_R[r][WIN_SIZE + MAX_DISP - 1] = val_R;
                if (r > 0) {
                    line_buf_L[r - 1][x] = val_L;
                    line_buf_R[r - 1][x] = val_R;
                }
            }
            win_L[WIN_SIZE - 1][WIN_SIZE - 1] = pix_L;
            win_R[WIN_SIZE - 1][WIN_SIZE + MAX_DISP - 1] = pix_R;
            line_buf_L[WIN_SIZE - 2][x] = pix_L;
            line_buf_R[WIN_SIZE - 2][x] = pix_R;

            // Parallel Sum of Absolute Differences (SAD) across MAX_DISP candidates
            ap_uint<16> best_sad = 0xFFFF;
            ap_uint<6>  best_d   = 0;

            if (y >= WIN_SIZE - 1 && x >= WIN_SIZE + MAX_DISP - 1) {
                disp_loop:
                for (int d = 0; d < MAX_DISP; d++) {
#pragma HLS UNROLL
                    ap_uint<16> sad = 0;
                    for (int r = 0; r < WIN_SIZE; r++) {
#pragma HLS UNROLL
                        for (int c = 0; c < WIN_SIZE; c++) {
#pragma HLS UNROLL
                            // Right camera correspondence shifts left by disparity d
                            int r_col = (WIN_SIZE + MAX_DISP - 1) - d - (WIN_SIZE - 1 - c);
                            sad += abs_diff_u8(win_L[r][c], win_R[r][r_col]);
                        }
                    }
                    if (sad < best_sad) {
                        best_sad = sad;
                        best_d   = (ap_uint<6>)d;
                    }
                }
            }

            // Brightness threshold tracking (matching Python cv2.threshold > thresh)
            if (pix_L > thresh) {
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
                area_cnt++;
                disp_acc += best_d;
            }

            // Stream out 8-bit scaled disparity pixel for visualisation
            disp_axis_t pkt_out;
            pkt_out.data = (ap_uint<8>)(best_d << 3); // Scale 0..31 -> 0..248
            pkt_out.keep = -1;
            pkt_out.strb = -1;
            pkt_out.user = pkt_in.user;
            pkt_out.last = (y == IMG_HEIGHT - 1 && x == IMG_WIDTH - 1) ? 1 : 0;
            pkt_out.id   = 0;
            pkt_out.dest = 0;
            out_disp_stream.write(pkt_out);
        }
    }

    // Latch hardware results into AXI4-Lite registers at end of frame
    bbox_min_x  = (area_cnt > 0) ? min_x : (ap_uint<16>)0;
    bbox_max_x  = (area_cnt > 0) ? max_x : (ap_uint<16>)0;
    bbox_min_y  = (area_cnt > 0) ? min_y : (ap_uint<16>)0;
    bbox_max_y  = (area_cnt > 0) ? max_y : (ap_uint<16>)0;
    target_area = area_cnt;
    disp_sum    = disp_acc;
}
