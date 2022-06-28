import datetime
import socket
import cv2
import numpy
from PIL import Image
import io
from multiprocessing import Process, Queue
import threading

######################################################################################

class CamReceiver:
    def __init__(self, width=320, height=240):
        self.width = width
        self.height = height
        self.HOST = ""  # Standard loopback interface address (localhost)
        self.PORT = 12345  # Port to listen on (non-privileged ports are > 1023)
        self.ACK = bytearray([0x96])
        self.frames = Queue()
        self.is_connected = False
        self.process = None

    #"""
    def cam_server_proc(self):
        global is_connected

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.HOST, self.PORT))
            s.listen(5)

            print("Server Listening ", self.PORT)
            self.is_connected = True

            while self.is_connected:
                conn, addr = s.accept()
                print(f"Connected by {addr}")

                while self.is_connected:
                    chunks = []
                    # print("Request Capture!!")
                    conn.send(self.ACK)
                    while self.is_connected:
                        data = conn.recv(1024)
                        if data:
                            # print("Received", len(data))
                            chunks += list(data)
                            # print("Last(2) = %02x %02x" % (int(chunks[-2]), int(chunks[-1])))
                            if chunks[-1] == 0xD9 and chunks[-2] == 0xFF:
                                #print("Received EOF (%d)" % len(chunks))

                                stream = io.BytesIO(bytes(chunks))
                                img = Image.open(stream)
                                img = cv2.cvtColor(numpy.array(img), cv2.COLOR_RGB2BGR)
                                self.frames.put(img)
                                break

    def start(self):
        #process = Process(target=cam_server_proc, args=())
        self.process = threading.Thread(target=self.cam_server_proc, args=())
        self.process.start()
        #"""

    def stop(self):
        self.is_connected = False
        #self.process.join()

    def get(self, flag=True):
        return self.frames.get(flag)

    def empty(self):
        return self.frames.empty()

######################################################################################

def starttime():
    return datetime.datetime.now()

def endtime(stime=0, show=False):
    etime = datetime.datetime.now() - stime
    if show:
        print("Elpased(ms): " + str(int(etime.total_seconds() * 1000)))
    return int(etime.total_seconds()*1000)

if __name__ == "__main__":
    cam = CamReceiver()
    cam.start()

    stime = starttime()
    count = 0
    while cv2.waitKey(1) != 27:
        if not cam.empty():
            elapsed = endtime(stime) / 1000
            count += 1
            img = cam.get(True)

            # f = open("./captures/%d.jpeg" % count, "wb")
            # f.write(bytes(chunks))
            # f.close()

            cv2.putText(img, "FPS %0.2f" % (count / elapsed), (5, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 1, 2)
            cv2.imshow("Captured", img)

            # TODO: Segmentation

    cam.stop()
    cv2.destroyAllWindows()
