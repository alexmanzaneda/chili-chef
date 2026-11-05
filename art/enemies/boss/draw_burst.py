"""Round burst of fire of the boss (32x32, 6 frames), with the colors of its fireball.

    python art/enemies/boss/draw_burst.py

writes art/enemies/boss/burst_sheet.png. Then tools/make_boss_images.py copies it to the game.

It is what a fireball leaves when it hits, the puff of a flame thrown out of the oven and
the explosions of the boss when it is beaten: a white hot ball that grows, turns into a ring
of fire that opens and breaks into embers. Its edge is a bit ragged, with the dark outline
of the fireball.
"""

import math
import os
from PIL import Image

T = 32
OUT = os.path.dirname(os.path.abspath(__file__))

# The colors of the fireball (draw_oven.py): from the white core to the dark red, and its outline
F0, F1, F2, F3, F4 = (255, 246, 196, 255), (253, 214, 96, 255), (253, 160, 50, 255), (228, 92, 30, 255), (160, 36, 26, 255)
OUTLINE = (17, 8, 22, 255)

# Each frame: outer radius, inner radius (the hole of the ring, 0 for none), the colors from
# the inside out, and how broken the ring is (0 whole .. 1 only embers)
FRAMES = [
	(5.0, 0.0, [F0, F0, F1], 0.0),
	(8.5, 0.0, [F0, F1, F2], 0.0),
	(11.5, 0.0, [F1, F1, F2, F3], 0.0),
	(13.0, 5.5, [F3, F1, F2, F3], 0.0),
	(14.0, 9.0, [F3, F1, F2], 0.35),
	(14.5, 11.0, [F2, F3], 0.7),
]


def hash01(x, y, s):
	h = (x * 374761393 + y * 668265263 + s * 2147483647) & 0xFFFFFFFF
	h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
	return (h ^ (h >> 16)) / 0xFFFFFFFF


def frame(f):
	outer, inner, colors, broken = FRAMES[f]
	im = Image.new('RGBA', (T, T), (0, 0, 0, 0))
	px = im.load()
	inside = {}
	for y in range(T):
		for x in range(T):
			dx, dy = x + 0.5 - T / 2, y + 0.5 - T / 2
			r = math.hypot(dx, dy)
			angle = math.atan2(dy, dx)
			# A ragged edge: the radius changes a bit with the angle
			wobble = 0.8 * math.sin(angle * 7 + f) + 0.5 * math.sin(angle * 11 - 2 * f)
			if not (inner <= r <= outer + wobble):
				continue
			# A broken ring keeps only some chunks, of 3x3 pixels
			if broken > 0 and hash01(x // 3, y // 3, f) < broken:
				continue
			t = (r - inner) / max(outer + wobble - inner, 1.0)
			inside[(x, y)] = colors[min(len(colors) - 1, int(t * len(colors)))]
	for (x, y), c in inside.items():
		outside = [(x + dx, y + dy) not in inside for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
		r = math.hypot(x + 0.5 - T / 2, y + 0.5 - T / 2)
		# The outline only goes around the outside: the hole of the ring is still fire
		px[x, y] = OUTLINE if any(outside) and r > inner + 1.5 else c
	return im


def main():
	sheet = Image.new('RGBA', (T * len(FRAMES), T), (0, 0, 0, 0))
	for f in range(len(FRAMES)):
		sheet.alpha_composite(frame(f), (f * T, 0))
	sheet.save(os.path.join(OUT, 'burst_sheet.png'))
	print('wrote burst_sheet.png')


if __name__ == '__main__':
	main()
