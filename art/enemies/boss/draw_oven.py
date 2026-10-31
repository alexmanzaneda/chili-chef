"""Oven boss (48x48) - floats on blue gas flames and spits fire through its door.

Usage: python draw_oven.py   -> writes PNGs next to this script.
Palette taken from toaster-pxl.png / pea.png so the three enemies match.
"""
from PIL import Image
import os
import sys

# ---- palette (shared with toaster / pea) ----
CLEAR = (0, 0, 0, 0)
O = (17, 8, 22, 255)           # outline
M_HI = (229, 227, 228, 255)    # metal highlight
M_LT = (198, 197, 205, 255)    # metal base
M_MD = (147, 145, 156, 255)    # metal shadow
M_DK = (102, 98, 114, 255)     # metal deep shadow
WH = (253, 251, 247, 255)      # eye white / teeth
PUP = (137, 0, 0, 255)         # pupil
PANEL = (52, 46, 64, 255)
PANEL_LT = (78, 72, 92, 255)
# fire (orange) - matches chili / toast tones
F0 = (255, 246, 196, 255)
F1 = (253, 214, 96, 255)
F2 = (253, 160, 50, 255)
F3 = (228, 92, 30, 255)
F4 = (160, 36, 26, 255)
F5 = (92, 16, 24, 255)
# gas (blue) - the burner flame it hovers on
G0 = (232, 246, 255, 255)
G1 = (140, 200, 255, 255)
G2 = (70, 128, 236, 255)
G3 = (40, 66, 170, 255)
SHADOW = (17, 8, 22, 90)
KNOB_ON = (253, 160, 50, 255)
KNOB_ON_HI = (253, 230, 150, 255)
KNOB_OFF = (78, 72, 92, 255)

W = H = 48


def new(w=W, h=H):
    return Image.new('RGBA', (w, h), CLEAR)


def px(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height:
        im.putpixel((x, y), c)


def rect(im, x0, y0, x1, y1, c):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            px(im, x, y, c)


def outline(im):
    """1px outline around every opaque, non-outline pixel."""
    src = im.copy()
    for y in range(im.height):
        for x in range(im.width):
            if src.getpixel((x, y))[3] != 0:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                xx, yy = x + dx, y + dy
                if 0 <= xx < im.width and 0 <= yy < im.height:
                    p = src.getpixel((xx, yy))
                    if p[3] == 255 and p != O:
                        im.putpixel((x, y), O)
                        break


def stamp(im, x0, y0, rows, colors, flip=False):
    w = len(rows[0])
    for r, row in enumerate(rows):
        for c, ch in enumerate(row):
            if ch in colors:
                px(im, x0 + ((w - 1 - c) if flip else c), y0 + r, colors[ch])


# left eye; the right one is mirrored. Brow slopes down toward the nose.
EYE = [
    "###........",
    "#####......",
    ".#WW#####..",
    ".#WWWWWW###",
    ".#WWWWWRRW#",
    ".#WWWWWRRW#",
    "..########.",
]
EYE_HURT = [
    "###........",
    "#####......",
    ".#WW#####..",
    ".#WWW#W#W##",
    ".#WWWW#WWW#",
    ".#WWW#W#WW#",
    "..########.",
]
EYE_RAGE = [  # eyes squint while spitting
    "...........",
    "####.......",
    "#######....",
    ".##WW######",
    ".#WWWWWRRW#",
    "..########.",
    "...........",
]


def gas_flame(im, cx, top, length, frame):
    """Small blue jet pointing down from a nozzle centred on cx."""
    wob = (0, 1, 0, -1)[frame % 4]
    L = length + (frame % 2)
    for k in range(L):
        y = top + k
        t = k / L
        half = 2 if t < 0.45 else 1 if t < 0.8 else 0
        for dx in range(-half, half + 1):
            x = cx + dx + (wob if t > 0.6 else 0)
            edge = abs(dx) == half and half > 0
            c = G3 if edge else (G0 if t < 0.25 else G1 if t < 0.55 else G2)
            px(im, x, y, c)


def fire_window(im, x0, y0, x1, y1, frame, bright=False):
    """Fire gradient inside the door glass, hotter at the bottom."""
    h = y1 - y0
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            t = (y - y0) / max(h, 1)
            # flickering tongues: column-dependent offset
            t += ((x * 5 + frame * 3) % 7 - 3) * 0.035
            if bright:
                t += 0.3
            c = F5 if t < 0.2 else F4 if t < 0.4 else F3 if t < 0.6 else F2 if t < 0.8 else F1
            px(im, x, y, c)
    # bright licks
    for i, lx in enumerate(range(x0 + 2, x1, 5)):
        hh = 2 + (i + frame) % 3 + (2 if bright else 0)
        for k in range(hh):
            px(im, lx, y1 - k, F0 if k < hh // 2 else F1)


def body(im, oy, frame, lives=5, mode='idle'):
    bx0, by0, bx1, by1 = 6, 2 + oy, 41, 34 + oy
    # metal box
    rect(im, bx0, by0, bx1, by1, M_LT)
    rect(im, bx0 + 1, by0, bx1 - 1, by0, M_HI)              # top highlight
    rect(im, bx0, by0 + 1, bx0, by1 - 2, M_HI)              # left highlight
    rect(im, bx1 - 1, by0 + 1, bx1, by1 - 1, M_MD)          # right shade
    rect(im, bx0 + 1, by1 - 1, bx1 - 1, by1, M_MD)          # bottom shade
    rect(im, bx0 + 2, by1, bx1 - 1, by1, M_DK)
    for (cx, cy, sx, sy) in ((bx0, by0, 1, 1), (bx1, by0, -1, 1), (bx0, by1, 1, -1), (bx1, by1, -1, -1)):
        for (x, y) in ((cx, cy), (cx + sx, cy), (cx, cy + sy)):
            px(im, x, y, CLEAR)                              # rounded corners

    # control panel: knobs = lives
    py0 = by0 + 2
    rect(im, bx0 + 3, py0, bx1 - 3, py0 + 5, O)
    rect(im, bx0 + 4, py0 + 1, bx1 - 4, py0 + 4, PANEL)
    rect(im, bx0 + 4, py0 + 1, bx1 - 4, py0 + 1, PANEL_LT)
    for i in range(5):
        kx = bx0 + 5 + i * 5
        on = i < lives
        rect(im, kx + 1, py0 + 2, kx + 2, py0 + 3, KNOB_ON if on else KNOB_OFF)
        if on:
            px(im, kx + 1, py0 + 2, KNOB_ON_HI)
    # little thermometer LED
    led = (253, 70, 50, 255) if (frame % 2 == 0 or mode == 'attack') else PUP
    rect(im, bx1 - 7, py0 + 2, bx1 - 6, py0 + 3, led)

    # eyes
    ey = by0 + 9
    rows = {'idle': EYE, 'hurt': EYE_HURT, 'attack': EYE_RAGE}[mode]
    pal = {'#': O, 'W': WH, 'R': PUP}
    if mode == 'attack':
        pal['R'] = (220, 30, 30, 255)
    stamp(im, bx0 + 3, ey, rows, pal)
    stamp(im, bx1 - 13, ey, rows, pal, flip=True)

    # door = mouth; while attacking the lower lip drops like a jaw
    open_ = (0, 2, 3, 3)[frame % 4] if mode == 'attack' else 0
    dx0, dy0, dx1, dy1 = bx0 + 3, by0 + 18, bx1 - 3, by1 - 2
    # handle
    rect(im, dx0 + 6, dy0 - 2, dx1 - 6, dy0 - 2, O)
    rect(im, dx0 + 5, dy0 - 1, dx0 + 5, dy0 - 1, O)
    rect(im, dx1 - 5, dy0 - 1, dx1 - 5, dy0 - 1, O)
    rect(im, dx0 + 6, dy0 - 1, dx1 - 6, dy0 - 1, M_HI)
    # door frame
    dy1 += open_
    rect(im, dx0, dy0, dx1, dy1, O)
    rect(im, dx0 + 1, dy0 + 1, dx1 - 1, dy1 - 1, M_LT)
    rect(im, dx0 + 1, dy1 - 1, dx1 - 1, dy1 - 1, M_MD)
    rect(im, dx1 - 1, dy0 + 1, dx1 - 1, dy1 - 1, M_MD)
    # glass
    gx0, gy0, gx1, gy1 = dx0 + 3, dy0 + 3, dx1 - 3, dy1 - 3
    rect(im, gx0 - 1, gy0 - 1, gx1 + 1, gy1 + 1, O)
    if mode == 'hurt':
        rect(im, gx0, gy0, gx1, gy1, F5)
    else:
        fire_window(im, gx0, gy0, gx1, gy1, frame, bright=(mode == 'attack'))
    # teeth hanging from the top of the glass
    for tx in range(gx0 + 1, gx1 - 3, 6):
        for k, (a, b) in enumerate(((0, 4), (1, 3), (2, 2))):
            rect(im, tx + a, gy0 + k, tx + b, gy0 + k, WH)
        px(im, tx + 4, gy0, M_HI)
    # two bottom fangs
    for tx in (gx0 + 4, gx1 - 6):
        rect(im, tx, gy1, tx + 2, gy1, WH)
        px(im, tx + 1, gy1 - 1, WH)

    # side vent
    for i in range(3):
        rect(im, bx1 - 2, by0 + 19 + i * 3, bx1 - 1, by0 + 19 + i * 3, M_DK)

    # burner nozzles under the body
    for nx in (bx0 + 7, bx1 - 10):
        rect(im, nx, by1 + 1, nx + 3, by1 + 2, M_DK)
        rect(im, nx, by1 + 1, nx + 3, by1 + 1, M_MD)
    return (bx0, by1)


def shadow(im, frame, oy):
    w = 15 - oy                      # shadow shrinks as the oven rises
    cx, cy = 24, 46
    for y in (-1, 0, 1):
        half = w if y == 0 else w - 3
        for x in range(cx - half, cx + half):
            px(im, x, cy + y, SHADOW)


BOB = (0, 1, 2, 1)


def boss(frame=0, lives=5, mode='idle'):
    im = new()
    oy = BOB[frame % 4] if mode != 'attack' else (0, -1, 1, 1)[frame % 4]
    shadow(im, frame, oy)
    layer = new()
    bx0, by1 = body(layer, oy, frame, lives, mode)
    outline(layer)
    # gas jets drawn after the outline so they stay soft
    for nx in (bx0 + 7, 41 - 10):
        gas_flame(layer, nx + 1, by1 + 3, 5 + (mode == 'attack') * 2, frame + nx)
        gas_flame(layer, nx + 2, by1 + 3, 4, frame + nx + 1)
    if mode == 'hurt' and frame % 2 == 0:
        # white flash
        for y in range(H):
            for x in range(W):
                p = layer.getpixel((x, y))
                if p[3] == 255 and p != O:
                    layer.putpixel((x, y), WH)
    im.alpha_composite(layer)
    return im


# ---------------- fireball projectile (16x16) ----------------
def fireball(frame=0):
    im = new(16, 16)
    # round head on the right, tail to the left (spit direction = right)
    head = [(10, 8, 4.2)]
    for y in range(16):
        for x in range(16):
            dx, dy = x - 10.5, y - 7.5
            r = (dx * dx + dy * dy) ** 0.5
            tail = 0
            if x < 10:
                wig = ((x + frame * 2) % 4 - 1.5) * 0.4
                spread = 1.0 + (10 - x) * 0.18
                tail = abs(y - 7.5 + wig) < 3.6 - (10 - x) * 0.33 * spread * 0.6
            if r < 4.6 or tail:
                t = r / 4.6 if r < 4.6 else 1.0 + (10 - x) * 0.1
                c = F0 if t < 0.35 else F1 if t < 0.65 else F2 if t < 0.95 else F3 if t < 1.5 else F4
                px(im, x, y, c)
    outline(im)
    # sparks
    for (x, y) in ((1 + frame % 3, 3 + frame % 2), (2 + (frame + 1) % 3, 12 - frame % 2)):
        if im.getpixel((x, y))[3] == 0:
            px(im, x, y, F2)
    return im


def sheet(frames, w=W, h=H):
    s = new(w * len(frames), h)
    for i, f in enumerate(frames):
        s.paste(f, (w * i, 0))
    return s


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))
    boss(0).save(f'{out}/oven_boss.png')
    sheet([boss(f) for f in range(4)]).save(f'{out}/oven_boss_idle_sheet.png')
    sheet([boss(f, mode='attack') for f in range(4)]).save(f'{out}/oven_boss_attack_sheet.png')
    sheet([boss(f, mode='hurt') for f in range(2)]).save(f'{out}/oven_boss_hurt_sheet.png')
    sheet([boss(0, lives=l) for l in range(5, -1, -1)]).save(f'{out}/oven_boss_lives_5_to_0.png')
    sheet([fireball(f) for f in range(4)], 16, 16).save(f'{out}/fireball_sheet.png')
