#ifndef _LEVELS_INCLUDE
#define _LEVELS_INCLUDE


#include <vector>
#include <glm/glm.hpp>


// Everything that changes from one level to another. GameScene only knows the number
// of the level and reads the rest from here and from the level files.
// To add a level: add its line here and export <file>.txt and <file>-dynamic.txt
// (tools/tmx2level.py does it from the Tiled map).


// What depends on the tilesheet of each world (all of them have 8x8 tiles of 32 px).
// Tile ids are the ones Tiled shows: column + 8 * row, starting at 0
struct WorldInfo
{
	const char *tilesheet;             // Same image as the tilesheet of its maps
	const char *boxBreak;              // Animation of the box destroyed by the fire (tools/make_disintegrate.py)
	glm::ivec2 box, door, openDoor;    // Cells (column, row) of the box and the door
	std::vector<int> freeTiles;        // Background tiles the entities go through
};

static const WorldInfo KITCHEN = { "images/kitchen.png", "images/kitchen_box_break.png", { 3, 1 }, { 5, 2 }, { 6, 2 }, { 3 } };
static const WorldInfo FRIDGE = { "images/fridge.png", "images/fridge_box_break.png", { 6, 0 }, { 6, 4 }, { 6, 5 }, { 3, 4, 5, 11, 12, 13, 19, 20, 21 } };


struct LevelInfo
{
	const char *file;        // "levels/1-1" -> levels/1-1.txt + levels/1-1-dynamic.txt
	const WorldInfo *world;
};

static const LevelInfo LEVELS[] =
{
	{ "levels/1-1", &KITCHEN },
	{ "levels/1-2", &KITCHEN },
	{ "levels/2-1", &FRIDGE },
};

static const int NUM_LEVELS = sizeof(LEVELS) / sizeof(LEVELS[0]);


#endif // _LEVELS_INCLUDE
