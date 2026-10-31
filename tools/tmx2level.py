"""Converts a Tiled map (.tmx) into the two level files the game loads.

    python tools/tmx2level.py art/maps/1-2.tmx game/levels/1-2

writes game/levels/1-2.txt and game/levels/1-2-dynamic.txt.

- Tiles of objects.tsx (box, door, chef, toaster...) go to <name>-dynamic.txt.
- The tiles of every other layer go to <name>.txt. If several layers have a tile in
  the same cell, the layer drawn on top (the last one in Tiled) wins.

The values are the tile id inside its tileset (the first tile is 0, an empty cell -1),
the same as Tiled's "Export as CSV". TileMap adds 1 when it loads them.
"""

import os
import sys
import xml.etree.ElementTree as ET

OBJECTS_TILESET = 'objects.tsx'

# Image the game uses for each tileset of art/maps (paths relative to game/)
TILESHEETS = {
	'kitchen.tsx': 'images/kitchen.png',
	'fridge.tsx': 'images/fridge.png',
	'oven.tsx': 'images/oven.png',
}

GID_MASK = 0x1FFFFFFF  # Tiled stores the flip flags in the 3 highest bits


def write_level(path, width, height, tilesheet, cells):
	with open(path, 'w', newline='\r\n') as f:
		f.write('TILEMAP\n')
		f.write('%d %d \t\t\t\t\t\t\t-- Size of tile map in tiles\n' % (width, height))
		f.write('32 32 \t\t\t\t\t\t\t-- Tile size & block size\n')
		f.write('%s \t-- Tilesheet\n' % tilesheet)
		f.write('8 8 \t\t\t\t\t\t\t\t-- Number of tiles in tilesheet\n')
		for y in range(height):
			f.write('  '.join(str(cells[y * width + x]) for x in range(width)) + '\n')
	print('wrote', path)


def main():
	if len(sys.argv) != 3:
		sys.exit(__doc__)
	tmx, out = sys.argv[1], sys.argv[2]

	root = ET.parse(tmx).getroot()
	width, height = int(root.get('width')), int(root.get('height'))
	tilesets = sorted((int(t.get('firstgid')), os.path.basename(t.get('source'))) for t in root.findall('tileset'))

	# When objects.tsx grows, Tiled may add it to the map a second time, and the ids of
	# both copies overlap. Remove the extra <tileset> line of the .tmx (and fix its tiles)
	if [name for _, name in tilesets].count(OBJECTS_TILESET) != 1:
		sys.exit('The map must use objects.tsx exactly once (check the <tileset> lines of the .tmx)')

	world = [name for _, name in tilesets if name != OBJECTS_TILESET]
	if len(world) != 1 or world[0] not in TILESHEETS:
		sys.exit('The map must use objects.tsx and exactly one of: ' + ', '.join(TILESHEETS))

	static = [-1] * (width * height)
	dynamic = [-1] * (width * height)
	for layer in root.findall('layer'):
		data = layer.find('data')
		if data.get('encoding') != 'csv':
			sys.exit('Layer "%s": save the map with the CSV tile layer format' % layer.get('name'))
		gids = [int(v) & GID_MASK for v in data.text.replace('\n', '').split(',')]
		for i, gid in enumerate(gids):
			if gid == 0:
				continue
			firstgid, name = max(t for t in tilesets if t[0] <= gid)
			if name == OBJECTS_TILESET:
				dynamic[i] = gid - firstgid
			else:
				static[i] = gid - firstgid

	write_level(out + '.txt', width, height, TILESHEETS[world[0]], static)
	write_level(out + '-dynamic.txt', width, height, TILESHEETS[world[0]], dynamic)


if __name__ == '__main__':
	main()
