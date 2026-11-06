"""Builds the intro screens of the levels from the drawings of art/screens/.

    python tools/make_screens.py

writes game/images/ui/screen_<name>.png for every entry of INTROS (Levels.h says which
level shows each one), and the game over screen from art/screens/game-over.jpeg (screen_game_over.png).
For the life lost screen it writes the hat of art/screens/lost-life.jpeg (screen_life_hat.png) and
the "x" and the digits of its counter of lives in the font of tools/title_font.py (screen_life_x.png,
screen_life_<digit>.png).
It also writes the titles of the screens drawn by the game (screen_title_<name>.png) and, from the
screens edited by hand in art/screens/hand/, the same screens for the game with their paper
transparent: the game draws the floor of the world behind the screens of a world
(UIRenderer::drawBackdrop) and a plain paper color behind the others (UIRenderer::drawPaper).
Run it again after editing the drawings.

The drawings are big upscaled JPEGs, so each one is first reduced to its real pixel grid
(the median of every cell removes the JPEG noise). Then its parts are placed on a canvas of
180x195: the window (720x780) with a pixel of art every 4 pixels of window. The frame is
rebuilt to that size, because the drawings are not as tall as the window.
The game over drawing has letters of half a pixel, so it is read on its fine grid (320x240) and
placed on a canvas of 360x390: a pixel of art every 2 pixels of window.

Some screens are not drawn here but edited by hand in art/screens/hand/ (export the .xcf there):
the menu, three intros (kitchen 2, fridge 1, oven), world clear, continue and life lost, and the gems. The game draws over some of
them, so if what it points to moves, its places must follow it: the box of options in MenuScene.cpp,
the gems in WorldClearScene.cpp, the buttons and the arrow in ContinueScene.cpp and the hat in
LifeLostScene.cpp.
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
	('fridge_2', 'Gemini_Generated_Image_ertuvzertuvzertu.jpeg'),
	# Kitchen 2, fridge 1 and the oven are edited by hand (HAND)
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


def edges(image, border=0):
	"""The pixels of the edges of image, inside border."""
	h, w = image.shape[:2]
	return ([(y, x) for y in range(border, h - border) for x in (border, w - 1 - border)] +
	        [(y, x) for x in range(border, w - border) for y in (border, h - 1 - border)])


def without_paper(image, seeds=None, tolerance=10, border=0, specks=0):
	"""image (RGB or RGBA) as RGBA with its paper transparent: the pixels like the first seed reached
	from the seeds (by default the edges) without crossing anything else. border makes that many
	rows and columns of the edge transparent (an old frame) and starts from inside it; specks makes
	the loose bits of up to that many pixels transparent too (the specks of the menu paper)."""
	rgba = np.zeros(image.shape[:2] + (4,), np.uint8)
	rgba[..., :image.shape[2]] = image[..., :image.shape[2]]
	if image.shape[2] == 3:
		rgba[..., 3] = 255
	h, w = rgba.shape[:2]
	if seeds is None:
		seeds = edges(image, border)
	paper = rgba[seeds[0]][:3].astype(int)
	close = np.abs(rgba[..., :3].astype(int) - paper).max(axis=2) < tolerance
	clear = np.zeros((h, w), bool)
	if border:
		clear[:border] = clear[-border:] = True
		clear[:, :border] = clear[:, -border:] = True
	pending = list(seeds)
	while pending:
		y, x = pending.pop()
		if 0 <= y < h and 0 <= x < w and close[y, x] and not clear[y, x]:
			clear[y, x] = True
			pending += [(y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)]
	if specks:
		seen = clear.copy()
		for start in zip(*np.where(~clear)):
			if seen[start]:
				continue
			piece, pending = [], [start]
			seen[start] = True
			while pending:
				y, x = pending.pop()
				piece.append((y, x))
				for ny, nx in ((y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)):
					if 0 <= ny < h and 0 <= nx < w and not seen[ny, nx]:
						seen[ny, nx] = True
						pending.append((ny, nx))
			if len(piece) <= specks:
				for y, x in piece:
					clear[y, x] = True
	rgba[clear] = 0
	return rgba


def make_intro(filename):
	source = native(os.path.join('art', 'screens', filename), GRID, GRID)
	blue = tuple(int(v) for v in np.median(source[185:190, 40:160].reshape(-1, 3), axis=0))
	canvas = framed_canvas(source, 10, 5, blue)

	paste(canvas, source, (28, 11, 173, 42), 18, 11, blue)  # Title, with room for the tail of the 'g'
	paste(canvas, source, (21, 43, 179, 179), 11, 47)       # Picture
	# The blue goes, inside the frame (the floor of the world is seen) and the strip around it
	return without_paper(canvas, seeds=[(8, 90)] + edges(canvas), tolerance=25)


# The screens edited by hand (art/screens/hand/screen_<name>.png): how their paper is found
HAND = {
	'menu': dict(border=4, specks=3),          # The old frame of the drawing and the specks of its paper go
	'world_clear': dict(),
	'life_lost': dict(),
	'continue': dict(),
	'oven': dict(seeds=[(8, 90)], tolerance=25, around=True),  # The blue inside the frame and around it, like the other intros
	'gem_green': dict(), 'gem_blue': dict(), 'gem_red': dict(),
	'continue_cursor': None,                     # Already transparent
	'kitchen_2': None, 'fridge_1': None,         # Intros edited by hand, already transparent
}
# The titles drawn by the game and their palette (tools/title_font.py)
TITLES = {'how_to_play': ('HOW TO PLAY', 'fire'), 'credits': ('CREDITS', 'fire'), 'continue': ('CONTINUE', 'fire'),
          'you_win': ('YOU WIN!', 'gold')}
LIFE_PALETTE = 'blue'  # The "x N" of the life lost screen, like its title


def make_hand(name):
	image = np.asarray(Image.open(os.path.join(ROOT, 'art', 'screens', 'hand', 'screen_' + name + '.png')).convert('RGBA'))
	if HAND[name] is None:
		return image
	options = dict(HAND[name])
	if options.pop('around', False):
		options['seeds'] = options['seeds'] + edges(image)
	return without_paper(image[..., :3], **options)


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
	return without_paper(canvas, tolerance=FINE_TOLERANCE)


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
	for name in HAND:
		save(make_hand(name), name)
	for name, (text, palette) in TITLES.items():
		save(render_title(text, palette), 'title_' + name)
	save(render_title('X', LIFE_PALETTE), 'life_x')
	for digit in '0123456789':
		save(render_title(digit, LIFE_PALETTE), 'life_' + digit)


if __name__ == '__main__':
	main()
