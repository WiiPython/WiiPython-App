class vec_2:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class pointer:
    def __init__(self, x, y, enable, angle):
        self.pos = vec_2(x, y)
        self.enable = enable
        self.angle = angle