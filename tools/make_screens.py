"""Builds the intro screens of the levels from the drawings of art/screens/.

    python tools/make_screens.py

writes game/images/ui/screen_<name>.png for every entry of INTROS (Levels.h says which
level shows each one). Run it again after editing the drawings.

The drawings are big upscaled JPEGs, so each one is first reduced to its real pixel grid
(the median of every cell removes the JPEG noise). Then its parts are placed on a canvas of
180x195: the window (720x780) with a pixel of art every 4 pixels of window. The frame is
rebuilt to that size, because the drawings are not as tall as the window.

The menu, the oven and the world clear screen are not built here: screen_menu.png, screen_oven.png
and screen_world_clear.png are exported by hand from their .xcf of art/screens/. MenuScene.cpp draws
the box with the options over the menu and WorldClearScene.cpp the screen_gem_<color>.png over the
gray gems, so if the gems move or change their shape its table must follow them.
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


def save(canvas, name):
	path = os.path.join(ROOT, 'game', 'images', 'ui', 'screen_' + name + '.png')
	Image.fromarray(canvas).save(path)
	print('wrote', os.path.normpath(path))


def main():
	for name, filename in INTROS:
		save(make_intro(filename), name)


if __name__ == '__main__':
	main()
