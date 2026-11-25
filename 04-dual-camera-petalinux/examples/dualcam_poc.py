# -*- coding: utf-8 -*-
import cv2
import numpy as np

def main():
    # 1. GStreamer pipeline for ZUBoard dual camera
    gstreamer_pipeline = (
        "v4l2src device=/dev/video0 io-mode=\"dmabuf\" ! "
        "video/x-raw, width=1280, height=480, format=BGR, framerate=60/1 ! "
        "appsink"
    )
    cap = cv2.VideoCapture(gstreamer_pipeline, cv2.CAP_GSTREAMER)

    if not cap.isOpened():
        print("Error: Cannot open /dev/video0 stream!")
        return

    print("Stream opened successfully.")

    # 2. Initialise StereoSGBM
    min_disp = 0
    num_disp = 32
    block_size = 5

    stereo = cv2.StereoSGBM_create(
        minDisparity=min_disp,
        numDisparities=num_disp,
        blockSize=block_size,
        P1=8 * 3 * block_size ** 2,
        P2=32 * 3 * block_size ** 2,
        disp12MaxDiff=1,
        uniquenessRatio=10,
        speckleWindowSize=100,
        speckleRange=32,
        mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY
    )

    baseline = 0.06
    focal_length = 700

    print("Starting ZUBoard stereo vision and tracking, press 'Ctrl + C' or 'q' to exit...")

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("Failed to grab frame")
                break

            # 3. Split side-by-side frame
            h, w_total, _ = frame.shape
            w_half = w_total // 2
            imgL = frame[:, :w_half]
            imgR = frame[:, w_half:]

            # Convert to grayscale for SGBM and tracking
            grayL = cv2.cvtColor(imgL, cv2.COLOR_BGR2GRAY)
            grayR = cv2.cvtColor(imgR, cv2.COLOR_BGR2GRAY)

            # 4. Compute disparity
            disparity = stereo.compute(grayL, grayR).astype(np.float32) / 16.0

            # 5. Brightness threshold tracking (adapted for monochrome sensor)
            _, mask = cv2.threshold(grayL, 200, 255, cv2.THRESH_BINARY)
            
            non_zero = cv2.countNonZero(mask)
            print(f"Mask non-zero pixels: {non_zero}")

            contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            
            if contours:
                c = max(contours, key=cv2.contourArea)
                area = cv2.contourArea(c)
                print(f"Detected object area: {area}")
                
                if area > 100:
                    x, y, w, box_h = cv2.boundingRect(c)
                    cv2.rectangle(imgL, (x, y), (x + w, y + box_h), (0, 255, 0), 2)

                    roi_disp = disparity[y:y+box_h, x:x+w]
                    valid_disp = roi_disp[roi_disp > 0]
                    if valid_disp.size > 0:
                        avg_disp = np.mean(valid_disp)
                        if avg_disp > 0:
                            depth = (focal_length * baseline) / avg_disp
                            cv2.putText(imgL, f"Depth: {depth:.2f}m", (x, y - 10), 
                                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

            # 6. Visualise results
            disp_vis = cv2.normalize(disparity, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_8U)
            disp_color = cv2.applyColorMap(disp_vis, cv2.COLORMAP_JET)

            cv2.imshow("ZUBoard Left Eye Tracking", imgL)
            cv2.imshow("ZUBoard Disparity Map", disp_color)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("\nStopping program gracefully via Ctrl + C...")

    finally:
        cap.release()
        cv2.destroyAllWindows()
        print("Camera resource released successfully.")

if __name__ == "__main__":
    main()
