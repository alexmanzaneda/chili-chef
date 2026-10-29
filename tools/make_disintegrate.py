"""Builds the animations of the things the fire destroys, from their own sprites.

    python tools/make_disintegrate.py

Everything comes apart little by little, like the soft blocks of Pocket Bomberman: holes
open all over it at the same time and grow, with a glowing edge where it is being eaten,
until only a few bits of the walls between them are left. Each animation has FRAMES cells
of 32x32; the first one is the sprite as it is.

- game/images/<world>_box_break.png: one row, the box of the tilesheet (must match
  BOX_BREAK_FRAMES of GameScene.cpp; the cells are the ones of Levels.h).
- game/images/<character>_death.png: one row for every frame of the enemy or the chef, so
  it comes apart from the pose it had when the fire hit it (must match DEATH_FRAMES of
  Enemy.cpp and deathRow() of each enemy, and BURN_FRAMES and burnRow() of Player.cpp).

Run it again after changing any of those sprites.
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')
T = 32
FRAMES = 6
CHAR_RANGE = 0.22  # How far (in "dissolve time") from the front the box changes color

# Progress of each frame: the pixels whose dissolve time is lower are already gone
THRESHOLDS = [0.0, 0.22, 0.4, 0.53, 0.63, 0.73]
NEIGHBOURS = ((1, 0), (-1, 0), (0, 1), (0, -1))

# world: tilesheet, box cell (column, row), color the box turns into, colors of the edge
WORLDS = {
    # Cardboard: it chars and burns with the colors of the chili explosion
    'kitchen': ('kitchen.png', (3, 1), (58, 32, 24), [(255, 128, 16), (255, 214, 64)]),
    # Ice: it melts, getting clearer, with white shining drops on the edge
    'fridge': ('fridge.png', (6, 0), (236, 248, 253), [(255, 255, 255), (98, 186, 211)]),
}

# character: spritesheet, cells (column, row) of all its frames, in the order of the rows of the output.
# All of them burn like the cardboard, with the colors of the chili explosion
CHARACTER_BURNT = (58, 32, 24)
CHARACTER_EDGE = [(255, 128, 16), (255, 214, 64)]
CHARACTERS = {
    'toaster': ('toaster.png', [(c, 0) for c in range(12)]),                       # 0-5 left, 6-11 right
    'pea': ('pea-anim.png', [(c, 0) for c in range(7)] + [(c, 0) for c in range(8, 15)]),  # 0-6 right, 7-13 left
    'chef': ('chef.png', [(c, r) for r in (0, 2, 4) for c in range(16)]),          # walk, jump, idle
}


def hash01(x, y, s):
    h = (x * 374761393 + y * 668265263 + s * 2147483647) & 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return (h ^ (h >> 16)) / 0xFFFFFFFF


def mix(a, b, t):
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(3))


def hole_seeds():
    """Where the holes start: one in each cell of a 3x3 grid, moved a bit at random,
    so they are spread over the whole box."""
    cell = T / 3
    return [((i + 0.25 + 0.5 * hash01(i, j, 3)) * cell, (j + 0.25 + 0.5 * hash01(i, j, 5)) * cell)
            for i in range(3) for j in range(3)]


SEEDS = hole_seeds()
MAX_DIST = T / 3 * 0.8  # Distance to the nearest seed of the last pixels to go


def dissolve_time(x, y):
    """When each pixel disappears (0 first, 1 last): holes open all over the box at the
    same time and grow, with small 2x2 bites so their border is ragged."""
    dist = min(((x + 0.5 - sx) ** 2 + (y + 0.5 - sy) ** 2) ** 0.5 for sx, sy in SEEDS)
    bites = hash01(x // 2, y // 2, 11)
    return 0.75 * min(1.0, dist / MAX_DIST) + 0.25 * bites


def luminance(c):
    return 0.3 * c[0] + 0.59 * c[1] + 0.11 * c[2]


def make_frame(box, f, burnt, edge_colors, outline):
    t = THRESHOLDS[f]
    progress = f / (FRAMES - 1)
    frame = Image.new('RGBA', (T, T), (0, 0, 0, 0))
    src, dst = box.load(), frame.load()

    alive = set()
    for y in range(T):
        for x in range(T):
            if src[x, y][3] >= 128 and dissolve_time(x, y) >= t:
                alive.add((x, y))

    # Loose pixels look like confetti: what is left must stay in one piece
    for _ in range(2):
        alive = {p for p in alive if sum((p[0] + dx, p[1] + dy) in alive for dx, dy in NEIGHBOURS) >= 2}

    for (x, y) in alive:
        # The border of every hole is the front that is eating the box
        front = f > 0 and any((x + dx, y + dy) not in alive for dx, dy in NEIGHBOURS)
        if front:
            # Glowing edge in chunks, with the dark outline between them so it reads as pixel art
            h = hash01(x // 2, y // 2, f)
            color = edge_colors[int(h * 2 * len(edge_colors))] if h < 0.5 else outline
        else:
            # The closer to the front and the later the frame, the more it changes color
            near = max(0.0, 1.0 - (dissolve_time(x, y) - t) / CHAR_RANGE) if f > 0 else 0.0
            amount = min(1.0, 0.2 * progress + 0.6 * near)
            color = mix(src[x, y][:3], burnt, amount)
        dst[x, y] = color + (255,)
    return frame


def make_strips(sheet, cells, burnt, edge_colors, out_name):
    """One row of FRAMES cells for each (column, row) of the 32x32 grid of the sheet."""
    image = Image.open(os.path.join(ROOT, 'game', 'images', sheet)).convert('RGBA')
    out = Image.new('RGBA', (T * FRAMES, T * len(cells)), (0, 0, 0, 0))
    for r, (col, row) in enumerate(cells):
        sprite = image.crop((col * T, row * T, (col + 1) * T, (row + 1) * T))
        colors = [c for _, c in sprite.getcolors(T * T) if c[3] >= 128]
        outline = min(colors, key=luminance)[:3]
        for f in range(FRAMES):
            out.paste(make_frame(sprite, f, burnt, edge_colors, outline), (f * T, r * T))
    path = os.path.join(ROOT, 'game', 'images', out_name)
    out.save(path)
    print('wrote', os.path.normpath(path))


def main():
    for world, (sheet, cell, burnt, edge_colors) in WORLDS.items():
        make_strips(sheet, [cell], burnt, edge_colors, world + '_box_break.png')
    for character, (sheet, cells) in CHARACTERS.items():
        make_strips(sheet, cells, CHARACTER_BURNT, CHARACTER_EDGE, character + '_death.png')


if __name__ == '__main__':
    main()
