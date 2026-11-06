"""Adds the wall tile with a snowflake to the fridge tilesheet.

    python tools/make_snowflake_tile.py

writes tile 7 of art/fridge/2Level.png and game/images/fridge.png (the same image).
It is the wall tile (tile 12) with the snowflake tinted over it, so it keeps its
stripes. The tint is very soft on purpose: a strong drawing would look like something
the player can pick up. Run it again after editing SNOWFLAKE or the tints.

The tile must be in freeTiles of FRIDGE (Levels.h) so the entities go through it.
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')
SHEETS = ['art/fridge/2Level.png', 'game/images/fridge.png']

TILE = 32
COLUMNS = 8
BACKGROUND_TILE = 12
SNOWFLAKE_TILE = 7

WHITE = (255, 255, 255)
# How much of WHITE each letter of the drawing gets: '+' softens the edges of the arms
TINTS = {'#': 0.55, '+': 0.28}

# Six arms, like a real snowflake, each one with a pair of branches
SNOWFLAKE = [
	'........#........',
	'........#........',
	'.....+#+#+#+.....',
	'....#.+###+.#....',
	'.##.#...#...#.##.',
	'..###...#...###..',
	'..+###+.#.+###+..',
	'.+#+.+#####+.+#+.',
	'.......###.......',
	'.+#+.+#####+.+#+.',
	'..+###+.#.+###+..',
	'..###...#...###..',
	'.##.#...#...#.##.',
	'....#.+###+.#....',
	'.....+#+#+#+.....',
	'........#........',
	'........#........',
]
SNOWFLAKE_POS = (7, 7)


def cell(tile):
	x, y = tile % COLUMNS * TILE, tile // COLUMNS * TILE
	return (x, y, x + TILE, y + TILE)


def draw_snowflake(background):
	tile = background.copy()
	for y, row in enumerate(SNOWFLAKE):
		for x, letter in enumerate(row):
			if letter in TINTS:
				pos = (SNOWFLAKE_POS[0] + x, SNOWFLAKE_POS[1] + y)
				r, g, b, alpha = background.getpixel(pos)
				t = TINTS[letter]
				tile.putpixel(pos, (round(r + (WHITE[0] - r) * t), round(g + (WHITE[1] - g) * t), round(b + (WHITE[2] - b) * t), alpha))
	return tile


for name in SHEETS:
	path = os.path.join(ROOT, name)
	sheet = Image.open(path).convert('RGBA')
	sheet.paste(draw_snowflake(sheet.crop(cell(BACKGROUND_TILE))), cell(SNOWFLAKE_TILE)[:2])
	sheet.save(path)
	print('wrote', name)
