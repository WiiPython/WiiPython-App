from . import screen
from . import textures
from .vector import vec_2, pointer
from . import util

import wiitools as w # type: ignore

pointer_pos = pointer(0, 0, False, 0.0)

def reset(app):
    print("reload123123")
    app.reload()

def update(app):
    global pointer_pos

    p = w.WPAD_IR(0)
    pointer_pos = pointer(p["x"], p["y"], p["valid"], p["angle"])

    app.button_pointer()
    
    

def render(app, no_pointer=False):
    global pointer_pos
    t = textures.get()

    w.draw_rect(0, 0, w.width, w.height, (21, 44, 82, 255), 0)
    w.draw_rect(0, 0, w.width, int(w.height/4), (255, 191, 0, 200), 0)

    icon_width = 100
    icon_height= 100

    w.png_use("icon")
    w.png_show_scaled(int(w.width/2 - icon_width/2), 10, 
                      icon_width, icon_height)

    size_text = 7
    w.render_text(int(w.width/4 - w.text_length("WII", size_text)/2), 30, "WII", 
                  size_text, True, (62, 201, 20, 255), 0)
    w.render_text(int(w.width*3/4 - w.text_length("Python", size_text)/2) + 20, 30, "Python", 
                  size_text, True, (73, 186, 99, 255), 0)

    app.render_app()

    if pointer_pos.enable and not no_pointer:
        w.png_use("pointer")
        util.png_rotate("pointer", pointer_pos.pos.x, pointer_pos.pos.y, pointer_pos.angle)


screen.init(screen.screen(update, render, reset, init=True), "main")