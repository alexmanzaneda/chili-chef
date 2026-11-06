"""Copies the drawings of the boss of art/enemies/boss/ to the images the game loads.

    python tools/make_boss_images.py

writes game/images/boss.png (boss-anim.png: cells of 42x42), game/images/fireball.png,
game/images/fire_burst.png (burst_sheet.png, drawn by art/enemies/boss/draw_burst.py) and
game/images/flame.png (flame_sheet.png, drawn by art/enemies/boss/draw_flame.py).
It also adds the boss and the flame to art/maps/objects.png (tiles 11 and 12 of objects.tsx).

Run it again after changing those drawings, and then tools/make_disintegrate.py and tools/make_hud.py.
"""

import os
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..')
ART = os.path.join(ROOT, 'art', 'enemies', 'boss')
IMAGES = os.path.join(ROOT, 'game', 'images')
OBJECTS = os.path.join(ROOT, 'art', 'maps', 'objects.png')
T = 32
FIRST_NEW_OBJECT = 11


def main():
	boss = Image.open(os.path.join(ART, 'boss-anim.png')).convert('RGBA')
	boss.save(os.path.join(IMAGES, 'boss.png'))

	Image.open(os.path.join(ART, 'fireball_sheet.png')).convert('RGBA').save(os.path.join(IMAGES, 'fireball.png'))
	Image.open(os.path.join(ART, 'burst_sheet.png')).convert('RGBA').save(os.path.join(IMAGES, 'fire_burst.png'))

	flame = Image.open(os.path.join(ART, 'flame_sheet.png')).convert('RGBA')
	flame.save(os.path.join(IMAGES, 'flame.png'))

	# The tiles of the map editor: the boss reduced to a tile, the flame as it is in the game
	objects = Image.open(OBJECTS).convert('RGBA')
	out = Image.new('RGBA', ((FIRST_NEW_OBJECT + 2) * T, T), (0, 0, 0, 0))
	out.paste(objects.crop((0, 0, FIRST_NEW_OBJECT * T, T)), (0, 0))
	out.paste(boss.crop((0, 0, 42, 42)).resize((T, T), Image.NEAREST), (FIRST_NEW_OBJECT * T, 0))
	out.paste(flame.crop((0, 0, T, T)), ((FIRST_NEW_OBJECT + 1) * T, 0))
	out.save(OBJECTS)
	print('wrote boss.png, fireball.png, fire_burst.png, flame.png and', os.path.normpath(OBJECTS))


if __name__ == '__main__':
	main()
