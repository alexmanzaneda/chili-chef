"""Draws art/banda/escudo.png, the golden shield of the 'Heart' power-up.

    python tools/draw_shield.py

It is pixel art written as text, so it can be retouched here or in any image
editor afterwards (then do not run this script again).
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')

COLORS = {
	'K': (60, 28, 8, 255),     # Outline
	'D': (196, 112, 18, 255),  # Gold in the shade
	'G': (240, 168, 36, 255),  # Gold
	'L': (255, 226, 112, 255), # Gold in the light
	'W': (255, 250, 214, 255), # Shine
	'r': (130, 18, 28, 255),   # Heart outline
	'R': (220, 44, 44, 255),   # Heart
	'P': (255, 150, 150, 255), # Heart shine
}

SHIELD = [
	'.KKKKKKKKKKKKKKK.',
	'KWWLLLLLLLLLLLLDK',
	'KWLLGGGGGGGGGGGDK',
	'KLLGGGGGGGGGGGGDK',
	'KLGGGGrrGrrGGGGDK',
	'KLGGGrPRrRRrGGGDK',
	'KLGGGrRRRRRrGGGDK',
	'KLGGGGrRRRrGGGGDK',
	'KLGGGGGrRrGGGGGDK',
	'KLGGGGGGrGGGGGGDK',
	'KLGGGGGGGGGGGGGDK',
	'.KLGGGGGGGGGGGDK.',
	'.KLGGGGGGGGGGGDK.',
	'..KLGGGGGGGGGDK..',
	'...KLGGGGGGGDK...',
	'....KLGGGGGDK....',
	'.....KLGGGDK.....',
	'......KLGDK......',
	'.......KKK.......',
]


def main():
	image = Image.new('RGBA', (len(SHIELD[0]), len(SHIELD)), (0, 0, 0, 0))
	for y, row in enumerate(SHIELD):
		assert len(row) == len(SHIELD[0]), 'row %d' % y
		for x, c in enumerate(row):
			if c != '.':
				image.putpixel((x, y), COLORS[c])
	image.save(os.path.join(ROOT, 'art/banda/escudo.png'))
	print('wrote art/banda/escudo.png', image.size)


if __name__ == '__main__':
	main()
