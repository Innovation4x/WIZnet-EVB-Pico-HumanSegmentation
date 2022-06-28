import datetime
import cv2
import numpy
from PIL import Image
from SelfieSegMP import SelfieSegMP
from CamReceiver import CamReceiver

def starttime():
    return datetime.datetime.now()

def endtime(stime=0, show=False):
    etime = datetime.datetime.now() - stime
    if show:
        print("Elpased(ms): " + str(int(etime.total_seconds() * 1000)))
    return int(etime.total_seconds()*1000)

width = 320
height = 240
seg = SelfieSegMP(width, height)
cam = CamReceiver(width, height)
cam.start()

# Load and resize the background image
bgd = cv2.imread('./images/green.png')
#bgd = cv2.imread('./images/background.jpeg')
bgd = cv2.resize(bgd, (width, height))

stime = starttime()
count = 0
while cv2.waitKey(1) != 27:
    if not cam.empty():
        elapsed = endtime(stime) / 1000
        count += 1
        img = cam.get(True)

        # TODO: Segmentation
        # Get segmentation mask
        mask = seg.seg(img)
        # Merge with background
        fg = cv2.bitwise_or(img, img, mask=mask)
        bg = cv2.bitwise_or(bgd, bgd, mask=~mask)
        img = cv2.bitwise_or(fg, bg)

        # Show image
        cv2.putText(img, "FPS %0.2f" % (count / elapsed), (5, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2, 2)
        cv2.imshow("Captured", img)

cam.stop()
cv2.destroyAllWindows()
