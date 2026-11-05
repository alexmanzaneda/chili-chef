"""Draws the switched off heating bars of the oven from the glowing ones.

    python tools/make_oven_unlit.py

The oven tilesheet has two glowing bars: a horizontal one (columns 0-5, rows 5-7) and a
vertical one (columns 7-9, rows 2-7). This writes each of them switched off UNLIT_ROWS rows
below, so every unlit tile is the id of its lit tile + UNLIT_ROWS * 16. The maps use the unlit
bars and the game draws the lit tiles over them while a bar is on.

An unlit tile is the floor (tile 86) with the bar in cold metal: the glow goes away, the tube
turns grey inside its dark outline, and the clamps lose the red of the glow.

It writes the same tiles into game/images/oven.png (the game) and art/oven/oven.png (Tiled).
Run it again after exporting oven.png from oven.xcf, if the .xcf does not have them.

It also writes game/images/oven_tubes.png, of the same size as the tilesheet: only the tubes
of the lit bars, in their cells, and the rest transparent. The game draws them over the bar
while it warns, so the tube heats up without the glow (and the floor) changing around it.
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')
SHEETS = ['game/images/oven.png', 'art/oven/oven.png']
TUBES = 'game/images/oven_tubes.png'
T = 32
FLOOR = (6, 5)    # Cell (column, row) of tile 86, the plain floor
BARS = [((0, 5), (6, 3)), ((7, 2), (3, 6))]  # First cell and size in cells of each lit bar
UNLIT_ROWS = 8

# Kinds of pixel of a lit bar. The glow over the floor is red with some green (31-51); the
# outline of the tube is a deeper red with almost no green, its core orange and yellow
CORE_RED, CORE_GREEN, CORE_BLUE = 150, 54, 45
CORE_YELLOW = 180   # Green of the yellow middle of the tube, which has some blue too
OUTLINE_GREEN = 27
CLAMP_GREEN = 55    # The clamps are greys: more green than the glow, and more blue than the core
OUTLINE = (28, 24, 28, 255)
BLACK = (0, 0, 0, 255)
# Cold metal for the core of the tube, from the light of each pixel when it glowed
METAL = [(188, 182, 182, 255), (128, 122, 124, 255)]


def luminance(c):
	return 0.3 * c[0] + 0.59 * c[1] + 0.11 * c[2]


def kind(c):
	"""What a pixel of a lit bar is: 'black', 'core' or 'outline' (of the tube), 'clamp' or 'glow'."""
	if max(c[:3]) < 16 or (c[1] <= 8 and c[0] < 60):
		return 'black'  # Black lines (the ones of the clamps are a bit red with the glow)
	if c[0] >= CORE_RED and c[1] >= CORE_GREEN and (c[2] < CORE_BLUE or c[1] >= CORE_YELLOW):
		return 'core'
	if c[1] <= OUTLINE_GREEN and c[0] >= 100:
		return 'outline'
	if c[1] >= CLAMP_GREEN:
		return 'clamp'
	return 'glow'


def unlit(lit, floor):
	"""The block of tiles of a bar, switched off (the floor repeats every T pixels)."""
	src = lit.load()
	out = Image.new('RGBA', lit.size)
	dst = out.load()
	for y in range(lit.height):
		for x in range(lit.width):
			c = src[x, y]
			k = kind(c)
			if k == 'black':
				dst[x, y] = BLACK
			elif k == 'core':
				dst[x, y] = METAL[0] if luminance(c) > 190 else METAL[1]
			elif k == 'outline':
				dst[x, y] = OUTLINE
			elif k == 'clamp':
				light = int(luminance(c))  # The same metal, without the red of the glow
				dst[x, y] = (light, int(light * 0.94), int(light * 0.94), 255)
			else:
				dst[x, y] = floor.getpixel((x % T, y % T))  # The glow goes away
	return out


def tube(lit):
	"""Only the glowing tube of the block of tiles of a bar."""
	src = lit.load()
	out = Image.new('RGBA', lit.size, (0, 0, 0, 0))
	dst = out.load()
	w, h = lit.size
	core = {(x, y) for y in range(h) for x in range(w) if kind(src[x, y]) == 'core'}
	for y in range(h):
		for x in range(w):
			# An outline pixel far from the core is a dark speck of the glow
			near_core = any((x + dx, y + dy) in core for dx in (-1, 0, 1) for dy in (-1, 0, 1))
			if (x, y) in core or (kind(src[x, y]) == 'outline' and near_core):
				dst[x, y] = src[x, y]
	return out


def main():
	for path in SHEETS:
		full = os.path.join(ROOT, path)
		sheet = Image.open(full).convert('RGBA')
		floor = sheet.crop((FLOOR[0] * T, FLOOR[1] * T, (FLOOR[0] + 1) * T, (FLOOR[1] + 1) * T))
		for (col, row), (cols, rows) in BARS:
			box = (col * T, row * T, (col + cols) * T, (row + rows) * T)
			sheet.paste(unlit(sheet.crop(box), floor), (col * T, (row + UNLIT_ROWS) * T))
		sheet.save(full)
		print('wrote', path)

	sheet = Image.open(os.path.join(ROOT, SHEETS[0])).convert('RGBA')
	tubes = Image.new('RGBA', sheet.size, (0, 0, 0, 0))
	for (col, row), (cols, rows) in BARS:
		box = (col * T, row * T, (col + cols) * T, (row + rows) * T)
		tubes.paste(tube(sheet.crop(box)), (col * T, row * T))
	tubes.save(os.path.join(ROOT, TUBES))
	print('wrote', TUBES)


if __name__ == '__main__':
	main()
