import os, sys
import wiitools as w # type: ignore
import files.screen as screen
import files.mmenu as main
from files import load_daemon
from files import util
from files import textures
from files.vector import vec_2, pointer
from files import selected_app

w.init()

argument = os.path.abspath(sys.argv[1])
os.chdir(argument)

button_width  = int(w.width/2 -100)
button_height = int(w.height/5)

class a:
    def __init__(self):
        self.quit = False
        self.apps = []
        self.side = 0
        self.reloaded = False
        self.load_angle = 0
        self.button_choosen = -1
        self.current_app = -1
    
    def reload(self):
        self.reloaded = False
        load_daemon.reload()

    def app_pos(self, num):
        match num % 4:
            case 0:
                return 50, w.height/4 + 50
            case 1:
                return 50, w.height - w.height/4 - 50
            case 2:
                return w.width/2 + 50, w.height/4 + 50
            case 3:
                return w.width/2 + 50, w.height - w.height/4 - 50
            
    def button_pointer(self):
        p = w.WPAD_IR(0)
        pos = pointer(p["x"], p["y"], p["valid"], p["angle"])

        self.button_choosen = -1
        if pos.enable:
            for i in range(self.side*4, min(len(self.apps), self.side*4 + 3)):
                x, y = self.app_pos(i)
                if (pos.pos.x > x and pos.pos.x < x + button_width and
                    pos.pos.y > y and pos.pos.y < y + button_height):
                    self.button_choosen = i
                    if w.WPAD_ButtonsDown(w.WPAD_BUTTON_A, 0):
                        self.current_app = i
                        screen.set("selected-app")


    def render_app(self):
        if self.reloaded:
            self.load_angle = 0
            for i in range(self.side*4, min(len(self.apps), self.side*4 + 3)):
                x, y = self.app_pos(i)

                if self.button_choosen == i:
                    y -= 5

                w.draw_oval(int(x), int(y), button_width, button_height, (98, 153, 217, 255), 0)

                if not self.apps[i].icon == None:            
                    w.png_use(self.apps[i].icon)
                    w.png_show_scaled(int(x), int(y), 128, 48)

                if not self.apps[i].name == None:
                    w.render_text(int(x) + 135, int(y) + 10, self.apps[i].name, 3, True, (255, 255, 255, 255), 0)

                if not self.apps[i].date == None:
                    w.render_text(int(x) + 15, int(y) + 50, self.apps[i].date, 2, True, (255, 255, 255, 255), 15)

                if not self.apps[i].folder == None:
                    w.render_text(int(x) + 128, int(y) + 70, self.apps[i].folder, 2, True, (255, 255, 255, 255), -5)
        else:
            self.load_angle += 20
            t = textures.get()
            w.png_use("load")
            util.png_rotate("load", 
                            int(w.width/2),
                            int((w.height + w.height/4)/2),
                            self.load_angle)
            




    class app_info:
        def __init__(self, name, icon, description, folder, main, owner, date):
            self.name = name
            self.icon = icon
            self.description = description
            self.folder = folder
            self.main = main
            self.owner = owner
            self.date = date



            
        



    

app = a()

def init():
    screen.set("main", app=app)

    w.rendering_init() # start display

    import files.textures as tex
    tex.load()
    load_daemon.init(app)

init()

while not app.quit:
    s = screen.get()

    s.update(app)
    s.render(app)

    w.input_update()
    w.render_update()