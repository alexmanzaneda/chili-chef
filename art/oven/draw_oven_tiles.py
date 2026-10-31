"""Oven world tileset (32x32 tiles, 8x8 grid -> oven_tiles.png).

Tile ids follow the fridge tileset (2Level.png) so maps can swap tilesets.
Glass blocks only have a frame on their outer sides, so neighbouring blocks
join into one pane (no seams between them):
  1-3     one-row platform, left / middle / right          (red-hot top)
  9-11    top row of a bigger block, left / middle / right (red-hot top)
  17-19   middle rows, left / middle / right
  25-27   bottom row, left / middle / right
  15/23/31  one-column pillar: top (red-hot) / middle / bottom
  16      single isolated block (red-hot top)
  7       destructible: slice of sandwich bread (variant: plain, toasted, burnt, cheese)
  4-6, 12-14, 20-22     oven interior background (9-slice, heating element on top)
  28-30, 36-38, 44-46   steel border frame (9-slice, 37 left empty)
  39 / 47 exit door closed / open (same door as the other worlds)

Usage: python draw_oven_tiles.py [out_dir]
"""
from PIL import Image
import os
import sys

T = 32
CLEAR = (0, 0, 0, 0)
O = (17, 8, 22, 255)
# heat
E0 = (255, 244, 190, 255)
E1 = (253, 208, 84, 255)
E2 = (244, 128, 42, 255)
E3 = (190, 56, 32, 255)
CHAR = (52, 32, 38, 255)
# oven enamel (background)
BG = (56, 48, 64, 255)
BG_SPK = (46, 39, 54, 255)
BG_SPK2 = (70, 62, 80, 255)
BG_DK = (36, 30, 44, 255)
BG_DD = (26, 20, 32, 255)
# steel (same as the oven boss / toaster)
S_HI = (229, 227, 228, 255)
S_LT = (198, 197, 205, 255)
S_MD = (147, 145, 156, 255)
S_DK = (102, 98, 114, 255)
S_DD = (66, 60, 78, 255)


def h(x, y, s=0):
    """Deterministic hash -> 0..255, used for speckles."""
    n = (x * 374761393 + y * 668265263 + s * 2147483647) & 0xffffffff
    n = (n ^ (n >> 13)) * 1274126177 & 0xffffffff
    return (n ^ (n >> 16)) & 0xff


def new(w=T, h_=T):
    return Image.new('RGBA', (w, h_), CLEAR)


def px(im, x, y, c):
    if 0 <= x < im.width and 0 <= y < im.height:
        im.putpixel((x, y), c)


def rect(im, x0, y0, x1, y1, c):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            px(im, x, y, c)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3)) + (255,)


# ---------------- glass blocks (oven door window) ----------------
GL_TOP = (118, 70, 52, 255)
GL_BOT = (70, 36, 40, 255)
FRAME = 4  # outline, light bevel, dark bevel, inner outline


def heat_top(im, seed, x0, x1):
    """Red-hot crust on platform tops (the fridge's snow equivalent)."""
    for y, c in enumerate((E0, E1, E1, E2, E3), start=1):
        rect(im, x0, y, x1, y, c)
    for x in range(x0, x1 + 1):
        if h(x, 0, seed) < 70:
            px(im, x, 1, E1)
        if h(x, 5, seed) < 40:
            px(im, x, 4, CHAR)
    # glowing drips hanging down
    x = x0 + 2 + seed % 4
    while x < x1 - 1:
        length = 2 + h(x, 9, seed) % 5
        for k in range(length):
            px(im, x, 6 + k, E2 if k < length - 1 else E3)
        if length > 3:
            px(im, x + 1, 6, E3)
        x += 4 + h(x, 7, seed) % 5


def glass_tile(U, D, L, R, seed):
    """One piece of a glass pane; U/D/L/R say which sides get a frame."""
    im = new()
    for y in range(T):
        # vertical position inside the whole pane: dark top, embers at the bottom
        ty = y / (T - 1)
        t = ty if (U and D) else ty * 0.5 if U else 0.5 + ty * 0.5 if D else 0.5
        for x in range(T):
            c = lerp(GL_TOP, GL_BOT, t)
            glow = max(0.0, t - 0.55) * 1.4
            if h(x // 2, y // 2, seed) < 60:
                glow *= 1.6
            c = lerp(c, E3, min(glow, 0.8))
            if t > 0.75 and h(x, y, seed + 1) < 5:
                c = E2
            im.putpixel((x, y), c)
    # diagonal reflections next to the left frame
    if L:
        y_start = 16 if U else 20
        for (off, w) in ((3, 2), (9, 1)):
            for k in range(14):
                for d in range(w):
                    x, y = FRAME + off + k + d - 2, y_start - k
                    if FRAME <= x < T and (FRAME if U else 0) <= y < T:
                        im.putpixel((x, y), lerp(im.getpixel((x, y)), (255, 240, 220, 255), 0.4))
    # frame on the outer sides only (mitred corners)
    for y in range(T):
        for x in range(T):
            sides = [(y, 'U')] * U + [(T - 1 - y, 'D')] * D + [(x, 'L')] * L + [(T - 1 - x, 'R')] * R
            if not sides:
                continue
            d, side = min(sides)
            if d >= FRAME:
                continue
            lit = side in 'UL'
            im.putpixel((x, y), (O, S_HI if lit else S_MD, S_LT if lit else S_DK, O)[d])
    if U:
        heat_top(im, seed, 1 if L else 0, T - 2 if R else T - 1)
    return im


# ---------------- destructible: slice of sandwich bread ----------------
# crust dark/mid/light, crumb shade/base/highlight
BREAD = {
    'plain':   ((122, 60, 34), (160, 88, 44), (200, 128, 60), (226, 196, 140), (246, 226, 180), (255, 244, 214)),
    'toasted': ((96, 44, 28), (138, 70, 36), (180, 104, 48), (204, 140, 70), (232, 178, 96), (248, 212, 140)),
    'burnt':   ((40, 26, 30), (64, 40, 38), (92, 58, 44), (96, 60, 42), (128, 80, 50), (164, 112, 70)),
    'cheese':  ((110, 52, 30), (150, 80, 40), (192, 116, 54), (214, 162, 92), (238, 200, 130), (252, 228, 170)),
}
GRILL = (150, 84, 40, 255)
CHEESE = ((214, 132, 30, 255), (250, 190, 50, 255), (255, 226, 110, 255), (255, 246, 196, 255))
SMOKE = ((120, 112, 128, 220), (160, 152, 166, 200))


def outline(im):
    src = im.copy()
    for y in range(im.height):
        for x in range(im.width):
            if src.getpixel((x, y))[3]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                xx, yy = x + dx, y + dy
                if 0 <= xx < im.width and 0 <= yy < im.height and src.getpixel((xx, yy))[3] == 255:
                    im.putpixel((x, y), O)
                    break


def bread_slice(variant='toasted'):
    """Front view of a sandwich-bread slice, square to fit a full 32x32 tile."""
    BR_DK, BR_MD, BR_LT, C_SH, C, C_HI = [c + (255,) for c in BREAD[variant]]

    # fills the whole tile so the chef can stand on it: flat top with
    # rounded corners, and a crown 1px wider than the body like real bread
    def inside(x, y):
        crown = 1 <= x <= 30 and 1 <= y <= 9 and (x, y) not in (
            (1, 1), (2, 1), (1, 2), (30, 1), (29, 1), (30, 2))
        body = 2 <= x <= 29 and 10 <= y <= 30 and (x, y) not in ((2, 30), (29, 30))
        return crown or body

    im = new()
    depth = {}
    for y in range(T):
        for x in range(T):
            if not inside(x, y):
                continue
            # depth into the slice: crust is the outer 3px
            d = min((abs(dx) + abs(dy) for dx in range(-4, 5) for dy in range(-4, 5)
                    if not inside(x + dx, y + dy)), default=9)
            depth[x, y] = d
            if d <= 3:
                # crust lit from the top-left
                lit = (x < 16 and y < 20) or y < 6
                dark = x > 26 or y > 26
                c = BR_LT if lit and d > 1 else BR_DK if dark else BR_MD
                if d == 1:
                    c = BR_MD if lit else BR_DK
            elif d == 4:
                c = C_SH
            else:
                # toasting browns the crumb more toward the edges
                c = C if d > 6 or variant == 'plain' else lerp(C, C_SH, 0.5)
                r = h(x, y, 41)
                if r < 16:
                    c = C_SH          # air holes
                elif r > 246:
                    c = C_HI
            im.putpixel((x, y), c)
    for (x, y) in ((10, 14), (19, 19), (14, 24), (22, 12)):
        px(im, x, y, C_SH)
        px(im, x + 1, y, C_SH)
        px(im, x, y - 1, C_HI)

    crumb = [k for k, d in depth.items() if d >= 4]
    if variant == 'toasted':
        # marks left by the oven rack
        for gy in (13, 19, 25):
            for (x, y) in crumb:
                if y in (gy, gy + 1) and (x + y) % 9 != 0:
                    im.putpixel((x, y), GRILL if y == gy else BR_MD)
    elif variant == 'burnt':
        # charred patches and still-glowing embers
        for (x, y) in crumb:
            # darker toward the edges, soft blotches in the middle
            d = depth[x, y]
            if d <= 5 or h(x // 2, y // 2, 51) < 50:
                im.putpixel((x, y), BR_MD if d > 5 else BR_DK)
            if h(x, y, 52) < 4:
                im.putpixel((x, y), E2)
        for (x, y), d in depth.items():
            if d == 2 and h(x, y, 53) < 30:
                im.putpixel((x, y), E3)
    elif variant == 'cheese':
        # melted gratin cheese over the top with drips
        drips = {6: 6, 9: 3, 13: 9, 17: 4, 21: 7, 25: 3}
        for (x, y), d in depth.items():
            edge = 12 + (2 if (x // 4) % 2 else 0)
            bottom = edge + drips.get(x, drips.get(x - 1, 0))
            if 2 <= d and y <= bottom:
                if y == bottom or d == 2:
                    c = CHEESE[0]
                elif y > edge:
                    c = CHEESE[1]
                else:
                    c = CHEESE[2] if h(x, y, 61) > 40 else CHEESE[3]
                im.putpixel((x, y), c)
        # browned gratin spots
        for (x, y) in ((8, 6), (15, 4), (20, 8), (11, 10), (24, 11)):
            px(im, x, y, CHEESE[0])
            px(im, x + 1, y, (176, 96, 34, 255))
    outline(im)
    if variant == 'burnt':
        # a little smoke wisp over the corner
        for (x, y), c in (((24, 1), 0), ((25, 0), 1), ((26, 1), 1), ((27, 0), 0), ((5, 1), 1), ((6, 0), 0)):
            if im.getpixel((x, y))[3] == 0:
                im.putpixel((x, y), SMOKE[c])
    return im


# ---------------- background (oven interior enamel) ----------------
def background_9slice():
    """96x96 region sliced into 9 tiles: edges darken toward the frame."""
    im = new(3 * T, 3 * T)
    for y in range(3 * T):
        for x in range(3 * T):
            # speckled enamel, tileable within the middle tile (period 32)
            r = h(x % T, y % T, 3)
            c = BG_SPK if r < 40 else BG_SPK2 if r > 236 else BG
            # warm glow rising from the bottom row of tiles
            if y >= 2 * T:
                t = (y - 2 * T) / T
                c = lerp(c, (110, 52, 40), t * 0.55)
            im.putpixel((x, y), c)
    # inner shadow along the outer edge of the region
    for d, c in ((0, BG_DD), (1, BG_DD), (2, BG_DK), (3, BG_DK)):
        for i in range(3 * T):
            for (x, y) in ((i, d), (d, i), (3 * T - 1 - d, i)):
                im.putpixel((x, y), c)
    for d, c in ((0, (64, 30, 30, 255)), (1, (84, 40, 34, 255))):
        for i in range(3 * T):
            im.putpixel((i, 3 * T - 1 - d), c)
    heating_element(im)
    return im


def heating_element(im):
    """Glowing coil running along the top tiles (ids 4-6)."""
    y0 = 9
    x_start, x_end = 6, 3 * T - 7
    # tube: outline, glow body, hot core
    rect(im, x_start, y0 - 2, x_end, y0 + 2, O)
    rect(im, x_start + 1, y0 - 1, x_end - 1, y0 + 1, E2)
    rect(im, x_start + 1, y0 - 1, x_end - 1, y0 - 1, E1)
    for x in range(x_start + 1, x_end):
        if (x // 3) % 2 == 0:
            px(im, x, y0, E1)
    # soft glow on the enamel below and above
    for x in range(x_start, x_end + 1):
        for dy, t in ((3, 0.45), (4, 0.25), (-3, 0.35)):
            y = y0 + dy
            im.putpixel((x, y), lerp(im.getpixel((x, y)), E3, t))
    # terminals into the left/right walls
    for x in (x_start - 2, x_end + 1):
        rect(im, x, y0 - 3, x + 1, y0 + 3, S_DK)
        rect(im, x, y0 - 3, x, y0 + 3, S_MD)
    # mounting brackets (one per tile so the top-middle tile still repeats)
    for bx in (T + 14, 14, 2 * T + 14):
        rect(im, bx, 3, bx + 3, y0 - 3, S_DK)
        rect(im, bx, 3, bx, y0 - 3, S_MD)
        rect(im, bx - 1, y0 - 2, bx + 4, y0 - 2, O)


# ---------------- steel border ----------------
# colour by depth from the outer edge of the frame (0 = outside, 31 = inner edge)
PROFILE = (
    [O, S_DD, S_DK, S_MD, S_HI, S_LT, S_MD]
    + [S_DK] * 16
    + [S_DD, S_MD, S_DD, S_DD, BG_DD, BG_DD, BG_DD, O, O]
)
# right/bottom bands face away from the light
SHADED = {S_HI: S_MD, S_LT: S_DK, S_MD: S_DD}


def border_9slice():
    N = 3 * T
    im = new(N, N)
    for y in range(N):
        for x in range(N):
            d = min(x, y, N - 1 - x, N - 1 - y)
            if d >= T:
                continue
            c = PROFILE[d]
            far = (N - 1 - x == d) or (N - 1 - y == d)
            if far and d < 7:
                c = SHADED.get(c, c)
            if 7 <= d <= 22:
                # brushed steel lines running along the band
                along = x if d in (y, N - 1 - y) else y
                if h(along % T, d, 11) < 30:
                    c = S_DD if far else S_MD
            im.putpixel((x, y), c)
    # rivets centred on the plate, every 16px so edge tiles repeat
    rv = 14
    for i in range(8, N, 16):
        for (cx, cy) in ((i, rv), (i, N - 1 - rv), (rv, i), (N - 1 - rv, i)):
            if min(cx, cy, N - 1 - cx, N - 1 - cy) != rv:
                continue
            rect(im, cx - 1, cy - 1, cx + 1, cy + 1, O)
            rect(im, cx - 1, cy - 1, cx, cy, S_LT)
            px(im, cx - 1, cy - 1, S_HI)
    # hollow centre
    rect(im, T, T, 2 * T - 1, 2 * T - 1, CLEAR)
    return im


def slice9(img):
    return [img.crop((c * T, r * T, c * T + T, r * T + T)) for r in range(3) for c in range(3)]


def main(out):
    sheet = new(8 * T, 8 * T)

    def put(tile_id, im):
        i = tile_id - 1
        sheet.paste(im, ((i % 8) * T, (i // 8) * T))

    # (U, D) frame per row of the block set, (L, R) per column
    rows = {1: (True, True), 9: (True, False), 17: (False, False), 25: (False, True)}
    cols = ((True, False), (False, False), (False, True))
    for first, (U, D) in rows.items():
        for c, (L, R) in enumerate(cols):
            put(first + c, glass_tile(U, D, L, R, first + c))
    for tid, (U, D) in ((15, (True, False)), (23, (False, False)), (31, (False, True)), (16, (True, True))):
        put(tid, glass_tile(U, D, True, True, tid))
    put(7, bread_slice('cheese'))
    for tid, t in zip((4, 5, 6, 12, 13, 14, 20, 21, 22), slice9(background_9slice())):
        put(tid, t)
    for tid, t in zip((28, 29, 30, 36, 37, 38, 44, 45, 46), slice9(border_9slice())):
        put(tid, t)
    # doors: shared with the other worlds
    here = os.path.dirname(os.path.abspath(__file__))
    fridge = Image.open(os.path.join(here, '..', 'fridge', '2Level.png')).convert('RGBA')
    for tid in (39, 47):
        i = tid - 1
        box = ((i % 8) * T, (i // 8) * T, (i % 8) * T + T, (i // 8) * T + T)
        sheet.paste(fridge.crop(box), box[:2])

    sheet.save(os.path.join(out, 'oven_tiles.png'))


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__)))
