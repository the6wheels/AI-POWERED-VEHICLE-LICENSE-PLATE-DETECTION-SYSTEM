import cv2
import libcamera
import numpy as np

def main():
    libcamera.start()
    camera_manager = libcamera.CameraManager()

    # Enumerate available cameras
    camera_manager.start()
    cameras = camera_manager.cameras()

    if not cameras:
        print("No cameras found.")
        return

    # Select the first camera
    camera = cameras[0]

    # Acquire the camera
    camera.acquire()

    # Configure camera settings (optional)
    # camera.control.set_exposure(libcamera.ExposureMode.Manual, 1000)
    # camera.control.set_gain(libcamera.GainMode.Manual, 1.0)

    # Start capturing frames
    stream = camera.streams()[0]
    buffer_count = 3  # Number of frames to buffer
    camera.allocate_buffers(stream, buffer_count)
    camera.start_all_streams()

    # Create a window to display the video feed
    cv2.namedWindow("Camera Feed", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Camera Feed", 640, 480)

    try:
        while True:
            # Wait for a frame from the camera
            frame = camera.get_frame(stream)

            # Convert the frame to a format compatible with OpenCV
            image = np.array(frame.buffer, copy=False)
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

            # Display the frame
            cv2.imshow("Camera Feed", image)

            # Check for key press to exit
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break
    finally:
        # Stop capturing frames and release the camera
        camera.stop_all_streams()
        camera.release()
        libcamera.stop()

        # Close the OpenCV window
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()

