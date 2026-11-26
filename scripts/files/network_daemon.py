import wiitools as w # type: ignore
import threading as thr
import time

class network:
    def __init__(self):
        self.ready = False

n = network()

def daemon():
    while True:
        if w.IsNetReady():
            n.ready = True
            break
        time.sleep(1)

    

    

def start():
    thr.Thread(target=daemon)