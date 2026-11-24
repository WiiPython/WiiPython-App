import os, sys
import wiitools as w
import runpy

w.init()

argument = os.path.abspath(sys.argv[1])
os.chdir(argument)

def clear():
  os.system("clear")

try:
  dir = os.listdir("apps")
except FileNotFoundError:
  dir = []

choosen = 0
i=0
go=True

while go:
  pt = []
  i=0
  clear()

  for app in dir:
    pt.append((">" if choosen == i else " ") + " " + app)

    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_A, 0):
      try:
        os.chdir(f"apps/{app}")
        runpy.run_path("main.py")
        clear()
        os.chdir(argument)

      except Exception as e:
        os.chdir(argument)
        print(f"Error running {app}: {e}")

    i += 1
  pressed = False
  for p in pt:
     print(p)

  while not pressed:
    w.input_update()
    w.VIDEO_WaitVSync()
    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_A, 0):
       pressed = True

    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_UP, 0):
        choosen = (choosen - 1) if choosen > 0 else choosen
        pressed = True

    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_DOWN, 0):
        choosen = (choosen + 1) % len(dir)
        pressed = True

    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_HOME, 0):
        go = False
        pressed = True