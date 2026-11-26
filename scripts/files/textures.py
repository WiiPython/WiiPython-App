import wiitools as w # type: ignore
from .vector import vec_2

class tex:
    def __init__(self, x, y):
        self.size = vec_2(x, y)


textures = {}

elements = [
    ["assets/icon.png", "icon"],
    ["assets/cursor_drag.png", "pointer-hold"],
    ["assets/cursor_pic.png", "pointer"],
    ["assets/throbber.png", "load"]
           ]

def load():
    global t
    for t in elements:
        x, y = w.png_load_named(t[0], t[1])
        textures[t[1]] = tex(x, y)

def get():
    return textures

