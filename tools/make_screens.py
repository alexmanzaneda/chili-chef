"""Builds the intro screens of the levels and the "world clear" screen from their drawings.

    python tools/make_screens.py

writes game/images/ui/screen_<name>.png for every entry of INTROS (Levels.h says which
level shows each one) and, from game/images/ui/stage-clear.jpeg, screen_world_clear.png
(with the gems in gray) and screen_gem_<color>.png (their places must match WorldClearScene.cpp).
Run it again after editing the drawings.

The drawings are big upscaled JPEGs, so each one is first reduced to its real pixel grid
(the median of every cell removes the JPEG noise). Then its parts are placed on a canvas of
180x195: the window (720x780) with a pixel of art every 4 pixels of window. The frame is
rebuilt to that size, because the drawings are not as tall as the window.

The menu and the oven are not built here: screen_menu.png and screen_oven.png are exported
by hand from their .xcf of art/screens/. MenuScene.cpp draws the box with the options over the menu.
"""

import os
import numpy as np
from PIL import Image

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

# The "world clear" canvas is 240x260: a pixel of art every 3 pixels of window. Boxes of the
# drawing in cells of its fine grid (194x288) and the place of each one in the canvas
WORLD_TITLE = ((10, 25, 186, 55), (32, 8))
WORLD_CHEF = ((53, 70, 150, 224), (76, 44))
GEMS = {
	'green': ((17, 208, 50, 244), (40, 182)),
	'blue': ((74, 228, 121, 280), (97, 202)),
	'red': ((145, 208, 178, 244), (168, 182)),
}


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


def framed_canvas(source, corner, thickness, background):
	"""A canvas filled with background and the frame of source: its corners and straight edges."""
	canvas = np.full((CANVAS_HEIGHT, CANVAS_WIDTH, 3), background, np.uint8)
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


def paste(canvas, source, box, x, y, background=None):
	"""Copies box (x0, y0, x1, y1) of source to (x, y). Pixels close to background become background."""
	x0, y0, x1, y1 = box
	part = source[y0:y1, x0:x1].copy()
	if background is not None:
		part[np.abs(part.astype(int) - background).max(axis=2) < 20] = background
	canvas[y:y + y1 - y0, x:x + x1 - x0] = part


def make_intro(filename):
	source = native(os.path.join('art', 'screens', filename), GRID, GRID)
	blue = tuple(int(v) for v in np.median(source[185:190, 40:160].reshape(-1, 3), axis=0))
	canvas = framed_canvas(source, 10, 5, blue)

	paste(canvas, source, (28, 11, 173, 42), 18, 11, blue)  # Title, with room for the tail of the 'g'
	paste(canvas, source, (21, 43, 179, 179), 11, 47)       # Picture
	return canvas


def make_world_clear():
	"""The canvas with the gems in gray, and the part of each gem in color."""
	DRAWING = os.path.join('game', 'images', 'ui', 'stage-clear.jpeg')
	WHITE = (255, 255, 255)
	fine = native(DRAWING, 194, 288)  # The dithering of the title needs the fine grid
	# The rest is drawn with cells twice as big, which are sharper read directly
	coarse = native(DRAWING, 97, 144).repeat(2, axis=0).repeat(2, axis=1)

	canvas = np.full((260, 240, 3), WHITE, np.uint8)
	paste(canvas, fine, WORLD_TITLE[0], *WORLD_TITLE[1], WHITE)
	paste(canvas, coarse, WORLD_CHEF[0], *WORLD_CHEF[1], WHITE)

	gems = {}
	for color, (box, place) in GEMS.items():
		x0, y0, x1, y1 = box
		gem = np.full((y1 - y0, x1 - x0, 3), WHITE, np.uint8)
		paste(gem, coarse, box, 0, 0, WHITE)
		gems[color] = gem
		gray = np.clip(gem.astype(float) @ (0.3, 0.59, 0.11), 0, 255).astype(np.uint8)
		canvas[place[1]:place[1] + y1 - y0, place[0]:place[0] + x1 - x0] = np.stack((gray,) * 3, axis=2)
	return canvas, gems


def save(canvas, name):
	path = os.path.join(ROOT, 'game', 'images', 'ui', 'screen_' + name + '.png')
	Image.fromarray(canvas).save(path)
	print('wrote', os.path.normpath(path))


def main():
	for name, filename in INTROS:
		save(make_intro(filename), name)
	canvas, gems = make_world_clear()
	save(canvas, 'world_clear')
	for color, gem in gems.items():
		save(gem, 'gem_' + color)


if __name__ == '__main__':
	main()
