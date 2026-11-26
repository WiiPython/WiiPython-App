class screen:
    def __init__(self, update, render, reset, init=False, app=None):
        self.update = update
        self.render = render
        self.reset  = reset
        self.screens= {}
        if not init:
            reset(app)

    def set(self, update, render, reset, init=False, app=None):
        self.update = update
        self.render = render
        self.reset  = reset
        if not init:
            reset(app)

s = screen(None, None, None, init=True)

def init(scr, name):
    global s
    s.screens[name] = scr


def set(name, app=None):
    global s
    print(s.screens)
    s.set(s.screens[name].update, 
          s.screens[name].render, 
          s.screens[name].reset, app=app)

def get():
    return s
