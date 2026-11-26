import wiitools as w # type: ignore
import math

from . import textures

def rotate_point(x, y, cx, cy, angle_deg):
    a = math.radians(angle_deg)
    dx, dy = x - cx, y - cy
    return (
        cx + dx * math.cos(a) - dy * math.sin(a),
        cy + dx * math.sin(a) + dy * math.cos(a),
    )

def png_rotate(png, cx, cy, angle):
    t = textures.get()
    h = t["pointer"].size.y
    wi= t["pointer"].size.x

    corners = [
        (cx - wi/2, cy - h/2),  
        (cx + wi/2, cy - h/2),  
        (cx + wi/2, cy + h/2),  
        (cx - wi/2, cy + h/2),  
    ]
    rotated = [rotate_point(x, y, cx, cy, angle) for x, y in corners]

    w.png_quad(rotated[0][0], rotated[0][1],
               rotated[1][0], rotated[1][1],
               rotated[2][0], rotated[2][1],
               rotated[3][0], rotated[3][1],
               0, 0, t[png].size.x, t[png].size.y)