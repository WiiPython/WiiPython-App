from . import screen
import wiitools as w # type: ignore
from .vector import vec_2, pointer
from .run import run

def reset(app):
    pass

button_width = int(w.width/3)
button_height= 80

button_back_x = 20
button_back_y = int(w.height - 20 - button_height)
button_run_x = w.width - 20 - button_width
button_run_y = button_back_y
button_text_size = 5

button_choosen = -1

pointer_pos = pointer(0, 0, False, 0.0)

def update(app):
    global pointer_pos, button_choosen
    
    p = w.WPAD_IR(0)
    pointer_pos = pointer(p["x"], p["y"], p["valid"], p["angle"])

    if (pointer_pos.pos.x > button_back_x and pointer_pos.pos.x < button_back_x + button_width and
        pointer_pos.pos.y > button_back_y and pointer_pos.pos.y < button_back_y + button_height):
        button_choosen = 0

    elif (pointer_pos.pos.x > button_run_x and pointer_pos.pos.x < button_run_x + button_width and
          pointer_pos.pos.y > button_run_y and pointer_pos.pos.y < button_run_y + button_height):
        button_choosen = 1
    else:
        button_choosen = -1

    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_A, 0):
        if button_choosen == 0:
            screen.set("main", app=app)
        elif button_choosen == 1:
            run(app)
            
    

def render(app):
    s = screen.get()

    s.screens["main"].render(app, no_pointer=True)
    w.draw_rect(0, 0, w.width, w.height, (83, 39, 161, 220), 0)

    pic_width  = 128*2
    pic_height = 48*2

    start_x = 20
    start_y = 20

    if not app.apps[app.button_choosen].icon == None:
        w.png_use(app.apps[app.button_choosen].icon)
        w.png_show_scaled(start_x, start_y, pic_width, pic_height)

    if not app.apps[app.button_choosen].name == None:
        w.render_text(start_x + pic_width + 10, start_y + 5,
                      app.apps[app.button_choosen].name,
                      6, True, (255, 255, 255, 255), 0)


    # [back]  [run]
    y_text_offset = 20
    w.draw_oval(button_back_x, button_back_y - (5 if button_choosen == 0 else 0), 
                button_width, button_height, (128, 20, 22, 200), 0)
    w.render_text(int(button_back_x + button_width/2 - w.text_length("Back", button_text_size)/2),
                  button_back_y + y_text_offset - (5 if button_choosen == 0 else 0), 
                  "Back", button_text_size, True, (255, 255, 255, 255), 0)

    w.draw_oval(button_run_x, button_run_y - (5 if button_choosen == 1 else 0), 
                button_width, button_height, (21, 209, 59, 200), 0)
    w.render_text(int(button_run_x + button_width/2 - w.text_length("Start", button_text_size)/2),
                  button_run_y + y_text_offset - (5 if button_choosen == 1 else 0), 
                  "Start", button_text_size, True, (255, 255, 255, 255), 0)

    
    
        



screen.init(screen.screen(update, render, reset, init=True), "selected-app")