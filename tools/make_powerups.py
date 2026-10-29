"""Builds the images of the power-ups from the drawings of art/.

    python tools/make_powerups.py

writes game/images/powerups.png and adds to art/maps/objects.png one "box with
power-up" tile per power-up, so they can be placed in Tiled. Run it again after
editing the .xcf files or adding a power-up.

Every power-up is a golden box with its drawing engraved in the middle: the
drawing is made smaller and painted with the colors of the box (dark outline,
light and dark gold), so all of them look alike whatever the original colors.

Layout of powerups.png (must match PowerUp.cpp): one row of 32x32 cells per
power-up, in the order of PowerUpType (PowerUp.h). Each row has POWER_UP_FRAMES
cells: the box, and the box with a glint so it shines in the game.

To add a power-up: add its drawing to POWER_UPS (same position as in PowerUpType),
add its "box with power-up" tile to objects.tsx and to LevelObject (GameScene.h).
"""

import os
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from xcf import read_xcf

ROOT = os.path.join(os.path.dirname(__file__), '..')

# Drawing of each power-up, in the order of PowerUpType: (.xcf of art/, part of it or None)
POWER_UPS = [
	('bomb/chili_bomb.xcf', (0, 0, 32, 32)),  # Bomb Up: the first frame of the bomb
	('banda/fuego.xcf', None),                # Fire Up
]

TILE = 32
POWER_UP_FRAMES = 2
EMBLEM_SIZE = 22  # The longest side of the drawing inside the box

# Colors of the box, from dark to light
INK = (66, 24, 6, 255)
RIM = (150, 62, 8, 255)
SHADOW = (206, 110, 14, 255)
GOLD = (240, 164, 28, 255)
PANEL = (252, 204, 56, 255)
PANEL_LIT = (255, 224, 104, 255)  # The panel in the frame that shines
LIGHT = (255, 240, 150, 255)

# objects.png: the box is cell 0 and the boxes with a power-up start at this cell
OBJECTS_BOX_CELL = 0
OBJECTS_FIRST_POWER_UP_CELL = 5
MARKER_SIZE = 16  # The power-up drawn over the box in Tiled


def golden_box():
	# Rounded black border, bevel (light top-left, dark bottom-right), an inner rim,
	# the panel and a rivet in each corner
	box = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
	px = box.load()
	last = TILE - 1
	for y in range(TILE):
		for x in range(TILE):
			if x in (0, last) and y in (0, last):
				continue
			ring = min(x, y, last - x, last - y)
			if ring == 0:
				color = INK
			elif ring <= 2:
				lit = (x <= 2 and y < last - 2) or (y <= 2 and x < last - 2)
				color = LIGHT if lit else SHADOW
			elif ring == 3:
				color = RIM
			else:
				color = PANEL
			px[x, y] = color
	for x, y in ((4, 4), (last - 4, 4), (4, last - 4), (last - 4, last - 4)):
		px[x, y] = LIGHT
	return box


def emblem(drawing):
	# The drawing made smaller, with only the colors of the box: the outline and the
	# darkest pixels in ink, the red ones in the rim color, the rest by its brightness
	drawing = drawing.crop(drawing.getbbox())
	scale = EMBLEM_SIZE / max(drawing.size)
	w, h = max(1, round(drawing.width * scale)), max(1, round(drawing.height * scale))
	small = drawing.resize((w, h), Image.LANCZOS).load()
	inside = [[small[x, y][3] >= 110 for x in range(w)] for y in range(h)]

	result = Image.new('RGBA', (w, h), (0, 0, 0, 0))
	px = result.load()
	for y in range(h):
		for x in range(w):
			if not inside[y][x]:
				continue
			r, g, b, _ = small[x, y]
			light = (r * 3 + g * 6 + b) / 10
			edge = any(not (0 <= x + dx < w and 0 <= y + dy < h and inside[y + dy][x + dx])
					   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
			if edge or light < 60:
				px[x, y] = INK
			elif r - g > 90:
				px[x, y] = RIM
			elif light > 170:
				px[x, y] = LIGHT
			elif light > 120:
				px[x, y] = GOLD
			else:
				px[x, y] = SHADOW
	return result


def glint(box):
	# The panel lights up, with a stripe of light in its top-left corner (not over the drawing)
	result = box.copy()
	px = result.load()
	for y in range(TILE):
		for x in range(TILE):
			if px[x, y] == PANEL:
				px[x, y] = LIGHT if 9 <= x + y <= 11 else PANEL_LIT
	return result


def power_up(path, part):
	drawing = read_xcf(os.path.join(ROOT, 'art', path))
	if part is not None:
		drawing = drawing.crop(part)
	box = golden_box()
	mark = emblem(drawing)
	box.alpha_composite(mark, ((TILE - mark.width) // 2, (TILE - mark.height) // 2))
	return box


def main():
	boxes = [power_up(path, part) for path, part in POWER_UPS]

	sheet = Image.new('RGBA', (POWER_UP_FRAMES * TILE, len(boxes) * TILE), (0, 0, 0, 0))
	for row, box in enumerate(boxes):
		sheet.alpha_composite(box, (0, row * TILE))
		sheet.alpha_composite(glint(box), (TILE, row * TILE))
	sheet.save(os.path.join(ROOT, 'game/images/powerups.png'))

	# Tiled: the box of the level with the power-up small in its bottom-right corner
	objects_path = os.path.join(ROOT, 'art/maps/objects.png')
	objects = Image.open(objects_path).convert('RGBA')
	width = (OBJECTS_FIRST_POWER_UP_CELL + len(boxes)) * TILE
	result = Image.new('RGBA', (max(width, objects.width), TILE), (0, 0, 0, 0))
	result.paste(objects.crop((0, 0, min(objects.width, OBJECTS_FIRST_POWER_UP_CELL * TILE), TILE)), (0, 0))
	level_box = objects.crop((OBJECTS_BOX_CELL * TILE, 0, (OBJECTS_BOX_CELL + 1) * TILE, TILE))
	for i, box in enumerate(boxes):
		cell = level_box.copy()
		cell.alpha_composite(box.resize((MARKER_SIZE, MARKER_SIZE), Image.NEAREST), (TILE - MARKER_SIZE, TILE - MARKER_SIZE))
		result.paste(cell, ((OBJECTS_FIRST_POWER_UP_CELL + i) * TILE, 0))
	result.save(objects_path)

	print('wrote game/images/powerups.png', sheet.size, 'and art/maps/objects.png', result.size)


if __name__ == '__main__':
	main()
