"""Builds the intro screens of the levels from the drawings of art/screens/.

    python tools/make_screens.py

writes game/images/ui/screen_<name>.png for every entry of INTROS (Levels.h says which
level shows each one), and the game over screen from art/screens/game-over.jpeg (screen_game_over.png).
For the life lost screen it writes the hat of art/screens/lost-life.jpeg (screen_life_hat.png) and
the "x" and the digits of its counter of lives in the font of tools/title_font.py (screen_life_x.png,
screen_life_<digit>.png).
Run it again after editing the drawings.

The drawings are big upscaled JPEGs, so each one is first reduced to its real pixel grid
(the median of every cell removes the JPEG noise). Then its parts are placed on a canvas of
180x195: the window (720x780) with a pixel of art every 4 pixels of window. The frame is
rebuilt to that size, because the drawings are not as tall as the window.
The game over drawing has letters of half a pixel, so it is read on its fine grid (320x240) and
placed on a canvas of 360x390: a pixel of art every 2 pixels of window.

Some screens are not built here, because they are edited by hand: the menu, the oven, world clear,
continue and life lost (screen_menu.png, screen_oven.png, screen_world_clear.png, screen_continue.png
with screen_continue_cursor.png, screen_life_lost.png). The game draws over some of them, so if
what it points to moves, its places must follow it: the box of options in MenuScene.cpp, the gems
in WorldClearScene.cpp, the arrow in ContinueScene.cpp and the hat in LifeLostScene.cpp.
"""

import os
import sys
import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from title_font import render as render_title

ROOT = os.path.join(os.path.dirname(__file__), '..')
CANVAS_WIDTH, CANVAS_HEIGHT = 180, 195
GRID = 200  # Real pixels of the side of the drawings

# Name of the output: screen_<name>.png, drawing of art/screens/
INTROS = [
	('kitchen_1', 'Kitchen - Level1.jpeg'),
	('kitchen_2', 'Kitchen - Level2.jpeg'),
	('fridge_1', 'Fridge - Level1.jpeg'),
	('fridge_2', 'Gemini_Generated_Image_ertuvzertuvzertu.jpeg'),
]


def native(filename, width, height):
	"""The drawing reduced to its real grid of width x height pixels."""
	image = np.asarray(Image.open(os.path.join(ROOT, filename)).convert('RGB'))
	rows, cols = image.shape[:2]
	result = np.zeros((height, width, 3), np.uint8)
	for y in range(height):
		for x in range(width):
			# Only the center of the cell: its borders are blurred by the upscaling
			cell = image[int((y + .25) * rows / height):int((y + .75) * rows / height) + 1,
			             int((x + .25) * cols / width):int((x + .75) * cols / width) + 1]
			result[y, x] = np.median(cell.reshape(-1, 3), axis=0)
	return result


def framed_canvas(source, corner, thickness, background, size=(CANVAS_WIDTH, CANVAS_HEIGHT)):
	"""A canvas filled with background and the frame of source: its corners and straight edges."""
	canvas = np.full((size[1], size[0], 3), background, np.uint8)
	rows, cols = source.shape[:2]
	# Each edge is the median of the whole side of the drawing, so it comes out straight
	canvas[:thickness, :] = np.median(source[:thickness, corner:cols - corner], axis=1, keepdims=True)
	canvas[-thickness:, :] = np.median(source[-thickness:, corner:cols - corner], axis=1, keepdims=True)
	canvas[:, :thickness] = np.median(source[corner:rows - corner, :thickness], axis=0, keepdims=True)
	canvas[:, -thickness:] = np.median(source[corner:rows - corner, -thickness:], axis=0, keepdims=True)
	canvas[:corner, :corner] = source[:corner, :corner]
	canvas[:corner, -corner:] = source[:corner, -corner:]
	canvas[-corner:, :corner] = source[-corner:, :corner]
	canvas[-corner:, -corner:] = source[-corner:, -corner:]
	return canvas


def paste(canvas, source, box, x, y, background=None, tolerance=20):
	"""Copies box (x0, y0, x1, y1) of source to (x, y). Pixels close to background become background."""
	x0, y0, x1, y1 = box
	part = source[y0:y1, x0:x1].copy()
	if background is not None:
		part[np.abs(part.astype(int) - background).max(axis=2) < tolerance] = background
	canvas[y:y + y1 - y0, x:x + x1 - x0] = part


def make_intro(filename):
	source = native(os.path.join('art', 'screens', filename), GRID, GRID)
	blue = tuple(int(v) for v in np.median(source[185:190, 40:160].reshape(-1, 3), axis=0))
	canvas = framed_canvas(source, 10, 5, blue)

	paste(canvas, source, (28, 11, 173, 42), 18, 11, blue)  # Title, with room for the tail of the 'g'
	paste(canvas, source, (21, 43, 179, 179), 11, 47)       # Picture
	return canvas


FINE_CANVAS = (360, 390)
# Its paper is so light that the white of the chef is only a bit lighter: a low tolerance keeps it
FINE_TOLERANCE = 10


def color(source, box):
	x0, y0, x1, y1 = box
	return tuple(int(v) for v in np.median(source[y0:y1, x0:x1].reshape(-1, 3), axis=0))


def make_game_over():
	source = native(os.path.join('art', 'screens', 'game-over.jpeg'), 320, 240)
	paper = color(source, (2, 2, 40, 8))
	canvas = np.full((FINE_CANVAS[1], FINE_CANVAS[0], 3), paper, np.uint8)

	# The arc of the title ends over the hat of the chef, so they are told apart by their
	# pieces: a piece that starts above row 110 belongs to the title. They are moved apart
	# to fill the taller canvas
	ink = np.abs(source.astype(int) - paper).max(axis=2) >= FINE_TOLERANCE
	title = np.zeros_like(ink)
	seen = np.zeros_like(ink)
	for start in zip(*np.where(ink)):
		if seen[start]:
			continue
		piece, pending = [], [start]
		seen[start] = True
		while pending:
			y, x = pending.pop()
			piece.append((y, x))
			for ny, nx in ((y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)):
				if 0 <= ny < 240 and 0 <= nx < 320 and ink[ny, nx] and not seen[ny, nx]:
					seen[ny, nx] = True
					pending.append((ny, nx))
		if min(y for y, _ in piece) < 110:
			for y, x in piece:
				title[y, x] = True
	for mask, dy in ((title, 51), (ink & ~title, 91)):
		part = np.where(mask[:, :, None], source, paper).astype(np.uint8)
		part[np.abs(part.astype(int) - paper).max(axis=2) < FINE_TOLERANCE] = paper
		ys, xs = np.where((part != paper).any(axis=2))
		canvas[ys + dy, xs + 20] = part[ys, xs]
	return canvas


# The hat of lost-life.jpeg, on its grid of 120x160. LifeLostScene.cpp draws it at x2
LIFE_HAT = (18, 130, 43, 152)  # The first hat, the one lit


def transparent_around(image, box):
	"""The part box of image as RGBA, with the paper connected to its edges made transparent.
	The hats are white too, so only the paper outside their outline goes."""
	x0, y0, x1, y1 = box
	part = image[y0:y1, x0:x1]
	h, w = part.shape[:2]
	light = part.min(axis=2) > 235
	outside = np.zeros((h, w), bool)
	pending = [(y, x) for y in range(h) for x in (0, w - 1)] + [(y, x) for x in range(w) for y in (0, h - 1)]
	while pending:
		y, x = pending.pop()
		if 0 <= y < h and 0 <= x < w and light[y, x] and not outside[y, x]:
			outside[y, x] = True
			pending += [(y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)]
	result = np.zeros((h, w, 4), np.uint8)
	result[..., :3] = part
	result[..., 3] = np.where(outside, 0, 255)
	return result


def make_life_hat():
	source = native(os.path.join('art', 'screens', 'lost-life.jpeg'), 120, 160)
	return transparent_around(source, LIFE_HAT)


def save(canvas, name):
	path = os.path.join(ROOT, 'game', 'images', 'ui', 'screen_' + name + '.png')
	# Unchanged images are not written again, so a viewer that has them open does not get in the way
	if os.path.exists(path) and np.array_equal(np.asarray(Image.open(path)), canvas):
		print('unchanged', os.path.normpath(path))
		return
	Image.fromarray(canvas).save(path)
	print('wrote', os.path.normpath(path))


def main():
	for name, filename in INTROS:
		save(make_intro(filename), name)
	save(make_game_over(), 'game_over')
	save(make_life_hat(), 'life_hat')
	save(render_title('X'), 'life_x')
	for digit in '0123456789':
		save(render_title(digit), 'life_' + digit)


if __name__ == '__main__':
	main()
