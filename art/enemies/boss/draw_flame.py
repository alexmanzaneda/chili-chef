"""Flame minion of the oven boss (32x32, 4 frames): a round drop of fire with angry eyes.

    python art/enemies/boss/draw_flame.py

writes art/enemies/boss/flame_sheet.png (the 4 frames in a row) and flame.png (the first one).
Then tools/make_boss_images.py copies it to the game.

The body is a few drops joined: a big one in the middle and two small tongues on its sides,
all of them round at the bottom. Each frame moves their tips a bit, so it flickers. The fire
goes in rings from the outside in (red, orange, yellow and an almost white core), with the
black outline of the other enemies.
"""

import math
import os
from PIL import Image

T = 32
FRAMES = 4
OUT = os.path.dirname(os.path.abspath(__file__))

BLACK = (12, 6, 8, 255)
RINGS = [(196, 38, 22, 255), (244, 112, 26, 255), (255, 196, 54, 255), (255, 242, 186, 255)]
EYE_WHITE = (255, 250, 236, 255)
PUPIL = (150, 20, 14, 255)

# Drops: center x, top y, radius of the round bottom, y of the center of that circle
DROPS = [(16.0, 2.0, 9.0, 21.5), (10.0, 9.0, 5.0, 22.0), (22.0, 8.0, 5.0, 22.0)]
# How each frame moves the tip of each drop (x, y)
SWAY = [[(-1, 0), (0, 1), (1, -1)], [(0, 1), (-1, 0), (0, 0)], [(1, 0), (0, -1), (-1, 1)], [(0, 1), (1, 0), (0, 0)]]


def in_drop(x, y, cx, top, r, cy, tip_dx):
	"""Inside a drop: a circle at the bottom and a point that narrows up to the tip."""
	px, py = x + 0.5, y + 0.5
	if py >= cy:
		return (px - cx) ** 2 + (py - cy) ** 2 <= r * r
	if py < top:
		return False
	t = (py - top) / (cy - top)        # 0 at the tip, 1 at the widest part
	center = cx + tip_dx * (1 - t) ** 2  # The tip leans; the bottom stays still
	return abs(px - center) <= r * t ** 0.75


def ring(x, y, frame):
	"""Index of the ring of fire of a pixel (0 the outer one), or -1 if it is outside."""
	best = -1
	for d, (cx, top, r, cy) in enumerate(DROPS):
		dx, dy = SWAY[frame][d]
		for k in range(len(RINGS) - 1, -1, -1):
			# Each ring is the same drop, smaller and lower
			if in_drop(x, y, cx, top + dy + 4.5 * k, r - 2.2 * k, cy + 1.2 * k, dx):
				best = max(best, k)
				break
	return best


def eyes(im):
	"""Like the toaster: white eyes with red pupils looking to the middle, outlined in black,
	and a frowning brow that goes down towards the middle."""
	px = im.load()
	for ex, inner_right in ((10, True), (18, False)):
		inner = (lambda i: i) if inner_right else (lambda i: 3 - i)  # 0 outer column, 3 inner one
		for i in range(4):
			x = ex + i
			for y in range(20, 23):
				px[x, y] = PUPIL if inner(i) >= 2 and y >= 21 else EYE_WHITE
			px[x, 23] = BLACK
			px[x, 19 if inner(i) >= 2 else 18] = BLACK  # The brow
			if inner(i) < 2:
				px[x, 19] = BLACK
		for y in range(19, 23):
			px[ex - 1, y] = BLACK
			px[ex + 4, y] = BLACK


def frame(f):
	im = Image.new('RGBA', (T, T), (0, 0, 0, 0))
	px = im.load()
	inside = {}
	for y in range(T):
		for x in range(T):
			k = ring(x, y, f)
			if k >= 0:
				inside[(x, y)] = k
	for (x, y), k in inside.items():
		edge = any((x + dx, y + dy) not in inside for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
		px[x, y] = BLACK if edge else RINGS[k]
	eyes(im)
	return im


def main():
	frames = [frame(f) for f in range(FRAMES)]
	sheet = Image.new('RGBA', (T * FRAMES, T), (0, 0, 0, 0))
	for f, im in enumerate(frames):
		sheet.alpha_composite(im, (f * T, 0))
	sheet.save(os.path.join(OUT, 'flame_sheet.png'))
	frames[0].save(os.path.join(OUT, 'flame.png'))
	print('wrote flame_sheet.png and flame.png')


if __name__ == '__main__':
	main()
