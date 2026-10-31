"""Builds the images of the power-ups from the drawings of art/.

    python tools/make_powerups.py

writes game/images/powerups.png and adds every power-up to art/maps/objects.png,
so they can be placed in Tiled. In the game a power-up of the map is hidden under
a box until the box is destroyed. Run it again after editing the .xcf files or
adding a power-up.

Every power-up is a box with its drawing engraved in the middle: the drawing is
made smaller and painted with the colors of the box (dark outline, light and dark
tones), so all of them look alike whatever the original colors. The boxes are
golden; the rare power-ups use the red palette.

Layout of powerups.png (must match PowerUp.cpp): one row of 32x32 cells per
power-up, in the order of PowerUpType (PowerUp.h). Each row has POWER_UP_FRAMES
cells: the box, and the box with a glint so it shines in the game.

To add a power-up: add its drawing to POWER_UPS (same position as in PowerUpType),
add its tile to objects.tsx and to LevelObject (GameScene.h).
"""

import os
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from xcf import read_drawing

ROOT = os.path.join(os.path.dirname(__file__), '..')


TILE = 32
POWER_UP_FRAMES = 2
EMBLEM_SIZE = 22  # The longest side of the drawing inside the box

# Colors of a box, from dark to light. 'lit' is the panel in the frame that shines
GOLD = {
	'ink': (66, 24, 6, 255),
	'rim': (150, 62, 8, 255),
	'shadow': (206, 110, 14, 255),
	'mid': (240, 164, 28, 255),
	'panel': (252, 204, 56, 255),
	'lit': (255, 224, 104, 255),
	'light': (255, 240, 150, 255),
}
RED = {
	'ink': (60, 8, 12, 255),
	'rim': (128, 20, 24, 255),
	'shadow': (180, 40, 34, 255),
	'mid': (222, 76, 48, 255),
	'panel': (240, 118, 72, 255),
	'lit': (250, 150, 100, 255),
	'light': (255, 206, 160, 255),
}

# Each power-up, in the order of PowerUpType: (.xcf or .png of art/, part of it or None, palette)
POWER_UPS = [
	('bomb/chili_bomb.xcf', (0, 0, 32, 32), GOLD),  # Bomb Up: the first frame of the bomb
	('banda/fuego.xcf', None, GOLD),                # Fire Up
	('banda/escudo.png', None, GOLD),               # Heart (tools/draw_shield.py)
	('banda/gorroxcf.xcf', None, GOLD),             # Extra life
	('banda/temporizador.xcf', None, GOLD),         # +30 seconds
	('banda/fuego.xcf', None, RED),                 # Full fire: like Fire Up, but red
]

# objects.png: the power-ups start at this cell
OBJECTS_FIRST_POWER_UP_CELL = 5


def box_image(colors):
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
				color = colors['ink']
			elif ring <= 2:
				lit = (x <= 2 and y < last - 2) or (y <= 2 and x < last - 2)
				color = colors['light'] if lit else colors['shadow']
			elif ring == 3:
				color = colors['rim']
			else:
				color = colors['panel']
			px[x, y] = color
	for x, y in ((4, 4), (last - 4, 4), (4, last - 4), (last - 4, last - 4)):
		px[x, y] = colors['light']
	return box


def emblem(drawing, colors):
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
				px[x, y] = colors['ink']
			elif r - g > 90:
				px[x, y] = colors['rim']
			elif light > 170:
				px[x, y] = colors['light']
			elif light > 120:
				px[x, y] = colors['mid']
			else:
				px[x, y] = colors['shadow']
	return result


def glint(box, colors):
	# The panel lights up, with a stripe of light in its top-left corner (not over the drawing)
	result = box.copy()
	px = result.load()
	for y in range(TILE):
		for x in range(TILE):
			if px[x, y] == colors['panel']:
				px[x, y] = colors['light'] if 9 <= x + y <= 11 else colors['lit']
	return result


def power_up(path, part, colors):
	drawing = read_drawing(os.path.join(ROOT, 'art', path))
	if part is not None:
		drawing = drawing.crop(part)
	box = box_image(colors)
	mark = emblem(drawing, colors)
	box.alpha_composite(mark, ((TILE - mark.width) // 2, (TILE - mark.height) // 2))
	return box


def main():
	boxes = [power_up(path, part, colors) for path, part, colors in POWER_UPS]

	sheet = Image.new('RGBA', (POWER_UP_FRAMES * TILE, len(boxes) * TILE), (0, 0, 0, 0))
	for row, box in enumerate(boxes):
		sheet.alpha_composite(box, (0, row * TILE))
		sheet.alpha_composite(glint(box, POWER_UPS[row][2]), (TILE, row * TILE))
	sheet.save(os.path.join(ROOT, 'game/images/powerups.png'))

	# Tiled: the power-up as it is seen in the game
	objects_path = os.path.join(ROOT, 'art/maps/objects.png')
	objects = Image.open(objects_path).convert('RGBA')
	width = (OBJECTS_FIRST_POWER_UP_CELL + len(boxes)) * TILE
	result = Image.new('RGBA', (max(width, objects.width), TILE), (0, 0, 0, 0))
	result.paste(objects.crop((0, 0, min(objects.width, OBJECTS_FIRST_POWER_UP_CELL * TILE), TILE)), (0, 0))
	for i, box in enumerate(boxes):
		result.paste(box, ((OBJECTS_FIRST_POWER_UP_CELL + i) * TILE, 0))
	result.save(objects_path)

	print('wrote game/images/powerups.png', sheet.size, 'and art/maps/objects.png', result.size)


if __name__ == '__main__':
	main()
