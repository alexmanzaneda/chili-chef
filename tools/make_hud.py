"""Builds the images of the HUD from the drawings of art/ (banda, bomb and enemies).

    python tools/make_hud.py

writes game/images/hud.png (atlas with the icons and the digits) and
game/images/band.png (a raised plate of the color of the layer 'Fondo' of art/banda.xcf).
Run it again after editing the .xcf files.

Layout of hud.png (must match the constants of HUD.cpp). Everything touches the
bottom of its cell, so the game aligns icons and numbers on the same baseline:
- Row 0: icons in 28x26 cells (the drawings scaled to fill them), bottom-centered: timer, hat, bomb, fire, shield ('Heart')
  and then one per enemy, in the order of EnemyType (Enemy.h): toaster, pea, boss, flame.
- Row 1 (y = 28): glyphs in 18x24 cells: 0-9 and ':'. They are the 5x7 font of
  UIRenderer.cpp at x3, white with a black shadow, so the game can tint them.

It also writes game/images/ui/lives.png (the hat) for the "life lost" screen.
"""

import os
import re
import sys
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(__file__))
from xcf import read_xcf, read_drawing

ROOT = os.path.join(os.path.dirname(__file__), '..')
# Drawing of each icon: (.xcf or .png of art/, part of it or None)
ICONS = [
	('banda/temporizador.xcf', None),
	('banda/gorroxcf.xcf', None),
	('bomb/chili_bomb.xcf', (0, 0, 32, 32)),  # The first frame of the bomb
	('banda/fuego.xcf', None),
	('banda/escudo.png', None),  # tools/draw_shield.py
	('enemies/toaster/toaster-pxl.xcf', None),
	('enemies/pea/pea-anim.xcf', (0, 0, 32, 32)),  # The first frame of the pea
	('enemies/boss/boss-anim.png', (0, 0, 42, 42)),
	('enemies/boss/old/flame_minion.png', None),
]
LIVES_ICON = 1  # Also saved alone for the "life lost" screen
ICON_W, ICON_H = 28, 26
SHIELD_ICON = 4  # A small badge over the hat, the only icon not as big as the others
SHIELD_SCALE = 26 / 32
ROW_GAP = 2  # Empty rows between icons and glyphs, so the scaled-up glyphs do not pick up pixels of the icons
GLYPHS = '0123456789:'
GLYPH_W, GLYPH_H, GLYPH_SCALE, SHADOW = 18, 24, 3, 1
BAND_W, BAND_H = 480, 40
BORDER, EDGE = 2, 2  # Pixels of the black border and of the bright and dark edges of the band

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


def fitted(icon, scale=None):
	"""The icon as big as its cell allows, so all of them look the same size, or at that scale.
	With hard edges: the shader discards the half-transparent pixels."""
	if scale is None:
		scale = min(ICON_W / icon.width, ICON_H / icon.height)
	small = icon.resize((round(icon.width * scale), round(icon.height * scale)), Image.LANCZOS)
	small.putalpha(small.getchannel('A').point(lambda a: 255 if a >= 128 else 0))
	return small.crop(small.getbbox())


def shade(color, amount):
	"""The color lighter (amount > 0) or darker (amount < 0), by that fraction."""
	target = 255 if amount > 0 else 0
	return tuple(int(c + (target - c) * abs(amount)) for c in color[:3]) + (255,)


def draw_band(fondo):
	"""The band as a raised plate, of the flat color of the layer 'Fondo': black border with the
	corners rounded and light from above (bright top and left edges, dark bottom and right ones)."""
	face = fondo.getpixel((BAND_W // 2, BAND_H // 2))
	band = Image.new('RGBA', (BAND_W, BAND_H), BLACK)
	draw = ImageDraw.Draw(band)
	left, top = BORDER, BORDER
	right, bottom = BAND_W - 1 - BORDER, BAND_H - 1 - BORDER

	# The edges, as whole frames from the outside in: first the dark one and the light one over it
	draw.rectangle((left + 1, top, right - 1, bottom), fill=shade(face, -0.45))
	draw.rectangle((left, top + 1, right, bottom - 1), fill=shade(face, -0.45))
	draw.rectangle((left + 1, top, right - EDGE, bottom - EDGE), fill=shade(face, 0.55))
	draw.rectangle((left, top + 1, right - EDGE, bottom - EDGE), fill=shade(face, 0.55))

	draw.rectangle((left + EDGE, top + EDGE, right - EDGE, bottom - EDGE), fill=face)
	return band


def main():
	glyphs = font()
	atlas = Image.new('RGBA', (max(len(ICONS) * ICON_W, len(GLYPHS) * GLYPH_W), ICON_H + ROW_GAP + GLYPH_H), (0, 0, 0, 0))

	icons = []
	for i, (path, part) in enumerate(ICONS):
		icon = read_drawing(os.path.join(ROOT, 'art', path))
		if part is not None:
			icon = icon.crop(part)
		icon = icon.crop(icon.getbbox())  # Without transparent rows, so it touches the baseline
		small = fitted(icon, SHIELD_SCALE if i == SHIELD_ICON else None)
		if small.width > ICON_W or small.height > ICON_H:
			sys.exit('%s is bigger than %dx%d' % (path, ICON_W, ICON_H))
		atlas.alpha_composite(small, (i * ICON_W + (ICON_W - small.width) // 2, ICON_H - small.height))
		icons.append(icon)

	# The glyph and its shadow end at the bottom row of the cell
	top = ICON_H + ROW_GAP + GLYPH_H - (7 * GLYPH_SCALE + SHADOW)
	for i, char in enumerate(GLYPHS):
		for dx, dy, color in ((SHADOW, SHADOW, BLACK), (0, 0, WHITE)):
			for row, bits in enumerate(glyphs[char]):
				for col in range(5):
					if bits & (0x10 >> col):
						for a in range(GLYPH_SCALE):
							for b in range(GLYPH_SCALE):
								atlas.putpixel((i * GLYPH_W + col * GLYPH_SCALE + a + dx, top + row * GLYPH_SCALE + b + dy), color)

	# Only the color of the background of banda.xcf: the icons and the numbers are drawn by the
	# game. The band of the game is lower than the drawing, so only its top rows are used
	band = read_xcf(os.path.join(ROOT, 'art/banda.xcf'), 'Fondo')
	if band.width != BAND_W or band.height < BAND_H:
		sys.exit('The layer Fondo of banda.xcf must be at least %dx%d' % (BAND_W, BAND_H))
	band = draw_band(band.crop((0, 0, BAND_W, BAND_H)))

	atlas.save(os.path.join(ROOT, 'game/images/hud.png'))
	band.save(os.path.join(ROOT, 'game/images/band.png'))
	icons[LIVES_ICON].save(os.path.join(ROOT, 'game/images/ui/lives.png'))
	print('wrote game/images/hud.png', atlas.size, ', game/images/band.png', band.size, 'and game/images/ui/lives.png')


if __name__ == '__main__':
	main()
