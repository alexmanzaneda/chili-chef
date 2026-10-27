"""Generates chili-explosion.png: 32x32 pixel-art pieces of a Bomberman-style cross.

Rows (pieces): 0 center, 1 horizontal arm, 2 vertical arm,
               3 end left, 4 end right, 5 end up, 6 end down
Columns (frames): 0 burst, 1 full, 2 full (flicker), 3 fading
"""
import math, sys
from PIL import Image

T = 32
OUT = sys.argv[1]

# Palette taken from chili_bomb.png plus fire tones
OUTLINE = (1, 0, 1, 255)
CRIMSON = (150, 6, 23, 255)     # #960617 (bomb sauce, dark)
RED     = (246, 7, 1, 255)      # #f60701 (bomb sauce)
ORANGE  = (255, 128, 16, 255)
YELLOW  = (255, 214, 64, 255)
WHITE   = (255, 250, 220, 255)
GLASS   = (218, 242, 248, 255)  # #daf2f8 (jar glass)
GLASS2  = (98, 186, 211, 255)   # #62bad3
SAUCE   = (198, 58, 80, 255)    # #c63a50

# Per frame: flame half-thickness, center radius, brightness offset
FRAMES = [
    dict(w=8.0,  r=18.0, bright=0.00, shards=False,  drops=False, phase=0.0),
    dict(w=12.0, r=21.0, bright=0.08, shards=False,  drops=False, phase=1.6),
    dict(w=11.0, r=20.0, bright=0.00, shards=False, drops=True,  phase=3.2),
    dict(w=7.0,  r=13.0, bright=-0.30, shards=False, drops=True, phase=4.8),
]


def hash01(x, y, s):
    h = (x * 374761393 + y * 668265263 + s * 2147483647) & 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return (h ^ (h >> 16)) / 0xFFFFFFFF


def seg_dist(px, py, ax, ay, bx, by):
    vx, vy = bx - ax, by - ay
    t = max(0.0, min(1.0, ((px - ax) * vx + (py - ay) * vy) / (vx * vx + vy * vy)))
    return math.hypot(px - (ax + t * vx), py - (ay + t * vy)), t


C = (T - 1) / 2.0  # 15.5


def field(piece, x, y, f):
    """Returns intensity (>0 inside the flame) for pixel (x, y)."""
    r = f['r']
    px, py = x, y
    def puff(t):
        return f['w'] * (1 + 0.18 * math.sin(2 * math.pi * t / 16.0 + f['phase']))
    w = f['w']
    vals = []
    if piece == 'center':
        vals.append(1 - math.hypot(px - C, py - C) / r)
        # Arms leave the center in the 4 directions with the same thickness as the arm pieces
        vals.append(1 - abs(py - C) / puff(px))
        vals.append(1 - abs(px - C) / puff(py))
        return max(vals)
    if piece == 'h':
        return 1 - abs(py - C) / puff(px)
    if piece == 'v':
        return 1 - abs(px - C) / puff(py)
    # Ends: the flame comes from the side that touches the previous tile and
    # tapers to a rounded tip near the other side
    tip = f['w'] * 0.8 + 2.0 # Rounded tip that ends ~2 px before the tile border
    if piece == 'left':   a, b, along = (T + 2, C), (tip, C), 'x'
    if piece == 'right':  a, b, along = (-3, C), (T - 1 - tip, C), 'x'
    if piece == 'up':     a, b, along = (C, T + 2), (C, tip), 'y'
    if piece == 'down':   a, b, along = (C, -3), (C, T - 1 - tip), 'y'
    d, t = seg_dist(px, py, a[0], a[1], b[0], b[1])
    # Same thickness as the arms where it connects, thinner at the tip
    thick = puff(px if along == 'x' else py) * (1 - 0.25 * max(0.0, (t - 0.6) / 0.4))
    return 1 - d / thick


def edge_fade(piece, x, y):
    """0 at the edges that connect with the neighbour tile, so the noise
    never breaks the seam between pieces."""
    dx = min(x, T - 1 - x) / 4.0
    dy = min(y, T - 1 - y) / 4.0
    if piece == 'h': return min(1.0, dx)
    if piece == 'v': return min(1.0, dy)
    if piece in ('left', 'right'): return min(1.0, dx)
    if piece in ('up', 'down'): return min(1.0, dy)
    return min(1.0, dx, dy)


PIECES = ['center', 'h', 'v', 'left', 'right', 'up', 'down']
sheet = Image.new('RGBA', (T * len(FRAMES), T * len(PIECES)), (0, 0, 0, 0))

for row, piece in enumerate(PIECES):
    for col, f in enumerate(FRAMES):
        tile = Image.new('RGBA', (T, T), (0, 0, 0, 0))
        val = [[0.0] * T for _ in range(T)]
        for y in range(T):
            for x in range(T):
                v = field(piece, x, y, f)
                # Flame-like jagged border: noise in 2x2 blocks, animated per frame
                n = hash01(x // 3, y // 3, col * 7 + 3) - 0.5
                v += n * 0.30 * edge_fade(piece, x, y)
                val[y][x] = v + f['bright'] * (1 if v > 0 else 0)
        for y in range(T):
            for x in range(T):
                v = val[y][x]
                if v <= 0:
                    # Outline: empty pixel touching the flame
                    if any(0 <= x + dx < T and 0 <= y + dy < T and val[y + dy][x + dx] > 0
                           for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                        tile.putpixel((x, y), OUTLINE)
                    continue
                if v > 0.90: c = WHITE
                elif v > 0.66: c = YELLOW
                elif v > 0.44: c = ORANGE
                elif v > 0.20: c = RED
                else: c = CRIMSON
                tile.putpixel((x, y), c)
        # Glass shards of the broken jar flying out of the center
        if piece == 'center' and f['shards']:
            spread = 13 if col == 0 else 15
            for k in range(4):
                ang = k * math.pi / 2 + math.pi / 4 + (0.25 if col else 0.0)
                sx = int(round(C + math.cos(ang) * spread)) - 1
                sy = int(round(C + math.sin(ang) * spread)) - 1
                for (ox, oy, cc) in ((0, 0, OUTLINE), (1, 0, GLASS), (2, 0, OUTLINE), (0, 1, GLASS), (1, 1, GLASS2), (2, 1, OUTLINE), (1, 2, OUTLINE)):
                    if 0 <= sx + ox < T and 0 <= sy + oy < T:
                        tile.putpixel((sx + ox, sy + oy), cc)
        # Chili sauce drops when the fire calms down
        if f['drops'] and piece in ('center', 'h', 'v'):
            for k in range(5):
                hx = int(hash01(k, row, col + 11) * 26) + 3
                hy = int(hash01(row, k, col + 29) * 26) + 3
                if tile.getpixel((hx, hy))[3] == 0:
                    tile.putpixel((hx, hy), SAUCE)
        sheet.paste(tile, (col * T, row * T))

sheet.save(OUT)
print('saved', OUT, sheet.size)
