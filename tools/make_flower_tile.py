"""Adds the background tile with a flower to the kitchen tilesheet.

    python tools/make_flower_tile.py

writes tile 28 of art/kitchen/1stWorld.png and game/images/kitchen.png (the same image).
It is the background tile (tile 3) with the flower tinted over it, so it keeps its
bevel and its grain. The tint is very soft on purpose: a strong drawing would look
like something the player can pick up. Run it again after editing FLOWER or the tint.

The tile must be in freeTiles of KITCHEN (Levels.h) so the entities go through it.
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')
SHEETS = ['art/kitchen/1stWorld.png', 'game/images/kitchen.png']

TILE = 32
COLUMNS = 8
BACKGROUND_TILE = 3
FLOWER_TILE = 28

BLUE = (168, 206, 248)
# How much of BLUE the drawing gets
TINT = 0.42
GLOSS = 0xe8  # Pixels with more blue than this are the shine of the tile, which stays over the flower

# The flower of the pavement of Barcelona: only the groove is drawn, the petals keep the color of the tile
FLOWER = [
	'..####.....####..',
	'.######...######.',
	'###...##.##...###',
	'##.....#.#.....##',
	'##.....#.#.....##',
	'##....#####....##',
	'.##..##...##..##.',
	'..####.....####..',
	'.....#.....#.....',
	'..####.....####..',
	'.##..##...##..##.',
	'##....#####....##',
	'##.....#.#.....##',
	'##.....#.#.....##',
	'###...##.##...###',
	'.######...######.',
	'..####.....####..',
]
FLOWER_POS = (7, 7)


def cell(tile):
	x, y = tile % COLUMNS * TILE, tile // COLUMNS * TILE
	return (x, y, x + TILE, y + TILE)


def draw_flower(background):
	tile = background.copy()
	for y, row in enumerate(FLOWER):
		for x, letter in enumerate(row):
			pos = (FLOWER_POS[0] + x, FLOWER_POS[1] + y)
			r, g, b, alpha = background.getpixel(pos)
			if letter == '#' and b <= GLOSS:
				tile.putpixel(pos, (round(r + (BLUE[0] - r) * TINT), round(g + (BLUE[1] - g) * TINT), round(b + (BLUE[2] - b) * TINT), alpha))
	return tile


for name in SHEETS:
	path = os.path.join(ROOT, name)
	sheet = Image.open(path).convert('RGBA')
	sheet.paste(draw_flower(sheet.crop(cell(BACKGROUND_TILE))), cell(FLOWER_TILE)[:2])
	sheet.save(path)
	print('wrote', name)
