import cv2
from PIL import Image
from libcamera import CameraManager

cm = CameraManager()
cm.start()
camera = cm.get('camera0')
camera.configure()
camera.start()

while True:
    frame = camera.get_frame()
    img = Image.frombytes('RGB', (frame.width, frame.height), frame.buffer)
    img = cv2.cvtColor(numpy.array(img), cv2.COLOR_RGB2BGR)
    cv2.imshow('Live Feed', img)
    
    if cv2.waitKey(1) == ord('q'):
        break

camera.stop()
cv2.destroyAllWindows()

