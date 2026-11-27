#include <iostream>
#include <cmath>
#include "stereo_tracker.hpp"

int main() {
    hls::stream<stereo_axis_t> in_stream;
    hls::stream<disp_axis_t>   out_disp_stream;

    const int EXPECTED_SHIFT = 14; // Simulated disparity = 14 px -> Depth = 700 * 0.06 / 14 = 3.00 m
    const int BOX_X0 = 300, BOX_X1 = 330;
    const int BOX_Y0 = 220, BOX_Y1 = 250;

    // Synthesise a 640x480 stereo frame with a bright target (intensity = 240)
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            ap_uint<8> pix_L = 20;
            ap_uint<8> pix_R = 20;

            if (y >= BOX_Y0 && y <= BOX_Y1 && x >= BOX_X0 && x <= BOX_X1) {
                pix_L = 205 + ((x + y) % 45);
            }
            if (y >= BOX_Y0 && y <= BOX_Y1 && (x + EXPECTED_SHIFT) >= BOX_X0 && (x + EXPECTED_SHIFT) <= BOX_X1) {
                pix_R = pix_R = 205 + (((x + EXPECTED_SHIFT) + y) % 45);
            }

            stereo_axis_t pkt;
            pkt.data.range(7, 0)  = pix_L;
            pkt.data.range(15, 8) = pix_R;
            pkt.keep = -1;
            pkt.strb = -1;
            pkt.user = (x == 0 && y == 0) ? 1 : 0;
            pkt.last = (x == IMG_WIDTH - 1 && y == IMG_HEIGHT - 1) ? 1 : 0;
            in_stream.write(pkt);
        }
    }

    ap_uint<16> min_x, max_x, min_y, max_y;
    ap_uint<32> area;
    ap_uint<64> disp_sum;

    stereo_tracker_accel(in_stream, out_disp_stream, 200, min_x, max_x, min_y, max_y, area, disp_sum);

    // Drain output disparity stream
    int out_count = 0;
    while (!out_disp_stream.empty()) {
        out_disp_stream.read();
        out_count++;
    }

    double avg_disp = (area > 0) ? (double)disp_sum / (double)area : 0.0;
    double depth_m  = (avg_disp > 0.0) ? (700.0 * 0.06) / avg_disp : 0.0;

    std::cout << "Processed pixels: " << out_count << std::endl;
    std::cout << "Detected BBox: [" << min_x << ", " << min_y << "] -> [" << max_x << ", " << max_y << "]" << std::endl;
    std::cout << "Target Area: " << area << " px, Mean Disparity: " << avg_disp << " px, Depth: " << depth_m << " m" << std::endl;

    if (min_x == BOX_X0 && max_x == BOX_X1 && std::fabs(avg_disp - EXPECTED_SHIFT) < 1.5) {
        std::cout << "PASS: Hardware disparity matches expected shift (" << EXPECTED_SHIFT << " px)" << std::endl;
        return 0;
    } else {
        std::cerr << "FAIL: Hardware disparity mismatch!" << std::endl;
        return 1;
    }
}
