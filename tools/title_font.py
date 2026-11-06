"""Pixel font of the big titles ("WORLD CLEAR!!", "YOU LOST A LIFE"), with the colors of GAME OVER.

    from title_font import render
    image = render('YOU LOST A LIFE')   # numpy RGBA image, 30 + 1 px high

It is the lettering of the world clear drawing rebuilt as a clean font: blocky letters 28 px high
with strokes of 4 px, a dark outline of 1 px and a shadow 1 px to the right and down. The color
goes in horizontal bands (red, a white stripe in the middle and red again), with a dithered
pink between them.
"""

import numpy as np

HEIGHT = 28  # Of the letters, without the outline
STROKE = 4
SPACE = 8    # Width of a space
GAP = 4      # Between letters: the outline and the shadow of both and a pixel of paper

# Colors of the GAME OVER title (screen_game_over.png)
OUTLINE = (7, 1, 33)
RED = (225, 48, 74)
PINK = (238, 143, 198)
WHITE = (255, 255, 255)
DARK_RED = (183, 47, 75)


def _letter(width, rects, holes=(), cut=()):
	"""Mask of a letter: the rectangles (x0, y0, x1, y1) filled, then the holes emptied and
	the corners in cut ('tl', 'tr', 'bl', 'br') rounded."""
	mask = np.zeros((HEIGHT, width), bool)
	for x0, y0, x1, y1 in rects:
		mask[y0:y1, x0:x1] = True
	for x0, y0, x1, y1 in holes:
		mask[y0:y1, x0:x1] = False
	w, h = width - 1, HEIGHT - 1
	for corner in cut:
		# Two pixels of the edge and one of the next row: a corner rounded in steps
		for dx, dy in ((0, 0), (1, 0), (0, 1)):
			x = dx if corner[1] == 'l' else w - dx
			y = dy if corner[0] == 't' else h - dy
			mask[y, x] = False
	return mask


S, H = STROKE, HEIGHT
FULL = (0, 0, 12, H)
INSIDE = (S, S, 12 - S, H - S)
MIDDLE = 12  # First row of the middle bar
GLYPHS = {
	'A': _letter(12, [FULL], [(S, S, 12 - S, MIDDLE), (S, MIDDLE + S, 12 - S, H)], 'tl tr'.split()),
	'C': _letter(12, [FULL], [INSIDE, (12 - S, MIDDLE - 4, 12, MIDDLE + 8)], 'tl tr bl br'.split()),
	'D': _letter(12, [FULL], [INSIDE], 'tr br'.split()),
	'E': _letter(12, [FULL], [(S, S, 12, MIDDLE), (S, MIDDLE + S, 12, H - S)]),
	'F': _letter(12, [FULL], [(S, S, 12, MIDDLE), (S, MIDDLE + S, 12, H)]),
	'I': _letter(S, [(0, 0, S, H)]),
	'L': _letter(11, [(0, 0, S, H), (0, H - S, 11, H)]),
	'O': _letter(12, [FULL], [INSIDE], 'tl tr bl br'.split()),
	'R': _letter(12, [FULL], [(S, S, 12 - S, MIDDLE), (S, MIDDLE + S, 12 - S, H)], 'tr'.split()),
	'S': _letter(12, [(0, 0, 12, S), (0, 0, S, MIDDLE + S), (0, MIDDLE, 12, MIDDLE + S),
	                  (12 - S, MIDDLE, 12, H), (0, H - S, 12, H), (12 - S, S, 12, S + 3), (0, H - S - 3, S, H - S)],
	             cut='tl tr bl br'.split()),
	'T': _letter(12, [(0, 0, 12, S), (S, 0, 12 - S, H)]),
	'U': _letter(12, [FULL], [(S, 0, 12 - S, H - S)], 'bl br'.split()),
	'W': _letter(16, [(0, 0, S, H), (16 - S, 0, 16, H), (6, MIDDLE, 10, H), (0, H - S, 16, H)], cut='bl br'.split()),
	'Y': _letter(12, [(0, 0, S, MIDDLE + 2), (12 - S, 0, 12, MIDDLE + 2), (0, MIDDLE - 2, 12, MIDDLE + 2), (S, MIDDLE, 12 - S, H)],
	             cut='bl br'.split()),
	'!': _letter(S, [(0, 0, S, H - 8), (0, H - S, S, H)]),
	# The X: two diagonal strokes, each row one step further (see below)
	'X': _letter(14, []),
	'0': _letter(12, [FULL], [INSIDE], 'tl tr bl br'.split()),
	'1': _letter(12, [(S, 0, 12 - S, H), (0, 0, S, S + 2), (0, H - S, 12, H)]),
	'2': _letter(12, [(0, 0, 12, S), (12 - S, 0, 12, MIDDLE + S), (0, MIDDLE, 12, MIDDLE + S),
	                  (0, MIDDLE, S, H), (0, H - S, 12, H), (0, S, S, S + 3)], cut='tl tr'.split()),
	'3': _letter(12, [(0, 0, 12, S), (12 - S, 0, 12, H), (S, MIDDLE, 12, MIDDLE + S), (0, H - S, 12, H)],
	             cut='tr br'.split()),
	'4': _letter(12, [(0, 0, S, MIDDLE + S), (12 - S, 0, 12, H), (0, MIDDLE, 12, MIDDLE + S)]),
	'5': _letter(12, [(0, 0, 12, S), (0, 0, S, MIDDLE + S), (0, MIDDLE, 12, MIDDLE + S),
	                  (12 - S, MIDDLE, 12, H), (0, H - S, 12, H), (0, H - S - 3, S, H - S)], cut='br bl'.split()),
	'6': _letter(12, [FULL], [(S, S, 12, MIDDLE), (S, MIDDLE + S, 12 - S, H - S)], 'tl tr bl br'.split()),
	'7': _letter(12, [(0, 0, 12, S), (12 - S, 0, 12, H)], cut='tr'.split()),
	'8': _letter(12, [FULL], [(S, S, 12 - S, MIDDLE), (S, MIDDLE + S, 12 - S, H - S)], 'tl tr bl br'.split()),
	'9': _letter(12, [FULL], [(S, S, 12 - S, MIDDLE), (0, MIDDLE + S, 12 - S, H - S)], 'tl tr bl br'.split()),
}
# The Y joins its arms with rounded corners under them
GLYPHS['Y'][MIDDLE + 1, 0] = GLYPHS['Y'][MIDDLE + 1, 11] = False
for _y in range(H):
	_x = round(_y * (14 - S) / (H - 1))
	GLYPHS['X'][_y, _x:_x + S] = True
	GLYPHS['X'][_y, 14 - S - _x:14 - _x] = True


def _band(y, x):
	"""Color of the pixel (x, y) of a letter: red, a dithered pink, a white stripe in the middle."""
	if y in (12, 13):
		return WHITE
	if y in (10, 11, 14, 15):
		return PINK if y in (11, 14) or (x + y) % 2 == 0 else RED
	if y in (8, 9, 16, 17):
		return PINK if (x + y) % 2 == 0 else RED
	if y >= H - 3:
		return DARK_RED
	return RED


def render(text):
	"""The text in one line as an RGBA image: transparent outside the letters, outline and shadow."""
	glyphs = [GLYPHS[c] if c != ' ' else None for c in text.upper()]
	width = sum(SPACE if g is None else g.shape[1] for g in glyphs) + GAP * (len(glyphs) - 1)
	letters = np.zeros((HEIGHT, width), bool)
	x = 0
	for g in glyphs:
		if g is None:
			x += SPACE + GAP
			continue
		letters[:, x:x + g.shape[1]] |= g
		x += g.shape[1] + GAP
	letters = letters[:, :x - GAP]

	# One pixel of outline around, and the shadow one more to the right and down
	h, w = letters.shape
	mask = np.zeros((h + 3, w + 3), bool)
	mask[1:h + 1, 1:w + 1] = letters
	dark = np.zeros_like(mask)
	for dy in (-1, 0, 1, 2):
		for dx in (-1, 0, 1, 2):
			if (dx, dy) == (2, -1) or (dx, dy) == (-1, 2) or (dx == 2 and dy == 2):
				continue
			dark |= np.roll(np.roll(mask, dy, axis=0), dx, axis=1)
	image = np.zeros((h + 3, w + 3, 4), np.uint8)
	image[dark] = OUTLINE + (255,)
	for y, x in zip(*np.where(mask)):
		image[y, x] = _band(y - 1, x - 1) + (255,)
	return image
