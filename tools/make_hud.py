"""Builds the images of the HUD from the drawings of art/banda.

    python tools/make_hud.py

writes game/images/hud.png (atlas with the icons and the digits) and
game/images/band.png (the layer 'Fondo' of art/banda.xcf). Run it again after editing the .xcf files.

Layout of hud.png (must match the constants of HUD.cpp). Everything touches the
bottom of its cell, so the game aligns icons and numbers on the same baseline:
- Row 0: icons in 34x32 cells, bottom-centered: timer, hat, chili, fire.
- Row 1 (y = 32): glyphs in 18x24 cells: 0-9 and ':'. They are the 5x7 font of
  UIRenderer.cpp at x3, white with a black shadow, so the game can tint them.

It also writes game/images/ui/lives.png (the hat) for the "life lost" screen.
"""

import os
import re
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from xcf import read_xcf

ROOT = os.path.join(os.path.dirname(__file__), '..')
ICONS = ['temporizador', 'gorroxcf', 'chili', 'fuego']
ICON_W, ICON_H = 34, 32
GLYPHS = '0123456789:'
GLYPH_W, GLYPH_H, GLYPH_SCALE, SHADOW = 18, 24, 3, 1
BAND_W, BAND_H = 480, 40

BLACK = (1, 0, 1, 255)
WHITE = (255, 255, 255, 255)


def font():
	src = open(os.path.join(ROOT, 'game/src/UIRenderer.cpp'), encoding='utf-8', errors='replace').read()
	glyphs = {}
	for name, first in (('FONT_DIGITS', '0'), ('FONT_SYMBOLS', None)):
		body = src[src.index(name):]
		body = body[body.index('{') + 1:body.index('};')]
		for i, g in enumerate(re.findall(r'\{([^{}]*)\}', body)):
			char = re.match(r"\s*'(.)'", g)
			values = [int(v, 0) for v in re.findall(r'0x[0-9A-Fa-f]+|\b\d+\b', g if char is None else g[char.end():])]
			glyphs[chr(ord(first) + i) if first else char.group(1)] = values[:7]
	return glyphs


def main():
	glyphs = font()
	atlas = Image.new('RGBA', (max(len(ICONS) * ICON_W, len(GLYPHS) * GLYPH_W), ICON_H + GLYPH_H), (0, 0, 0, 0))

	icons = {}
	for i, name in enumerate(ICONS):
		icon = read_xcf(os.path.join(ROOT, 'art/banda', name + '.xcf'))
		icon = icon.crop(icon.getbbox())  # Without transparent rows, so it touches the baseline
		if icon.width > ICON_W or icon.height > ICON_H:
			sys.exit('%s.xcf is bigger than %dx%d' % (name, ICON_W, ICON_H))
		atlas.alpha_composite(icon, (i * ICON_W + (ICON_W - icon.width) // 2, ICON_H - icon.height))
		icons[name] = icon

	# The glyph and its shadow end at the bottom row of the cell
	top = ICON_H + GLYPH_H - (7 * GLYPH_SCALE + SHADOW)
	for i, char in enumerate(GLYPHS):
		for dx, dy, color in ((SHADOW, SHADOW, BLACK), (0, 0, WHITE)):
			for row, bits in enumerate(glyphs[char]):
				for col in range(5):
					if bits & (0x10 >> col):
						for a in range(GLYPH_SCALE):
							for b in range(GLYPH_SCALE):
								atlas.putpixel((i * GLYPH_W + col * GLYPH_SCALE + a + dx, top + row * GLYPH_SCALE + b + dy), color)

	# Only the background of banda.xcf: the icons and the numbers are drawn by the game.
	# The band of the game is lower than the drawing, so only its top rows are used
	band = read_xcf(os.path.join(ROOT, 'art/banda.xcf'), 'Fondo')
	if band.width != BAND_W or band.height < BAND_H:
		sys.exit('The layer Fondo of banda.xcf must be at least %dx%d' % (BAND_W, BAND_H))
	band = band.crop((0, 0, BAND_W, BAND_H))

	atlas.save(os.path.join(ROOT, 'game/images/hud.png'))
	band.save(os.path.join(ROOT, 'game/images/band.png'))
	icons['gorroxcf'].save(os.path.join(ROOT, 'game/images/ui/lives.png'))
	print('wrote game/images/hud.png', atlas.size, ', game/images/band.png', band.size, 'and game/images/ui/lives.png')


if __name__ == '__main__':
	main()
