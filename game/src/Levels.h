#ifndef _LEVELS_INCLUDE
#define _LEVELS_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "Audio.h"


// Everything that changes from one level to another. GameScene only knows the number
// of the level and reads the rest from here and from the level files.
// To add a level: add its line here and export <file>.txt and <file>-dynamic.txt
// (tools/tmx2level.py does it from the Tiled map).


// What depends on the tilesheet of each world (all of them are square, with tiles of 32 px).
// Tile ids are the ones Tiled shows: column + columns of the sheet * row, starting at 0
struct WorldInfo
{
	const char *tilesheet;             // Same image as the tilesheet of its maps
	const char *boxBreak;              // Animation of the box destroyed by the fire (tools/make_disintegrate.py)
	glm::ivec2 box, door, openDoor;    // Cells (column, row) of the box and the door
	std::vector<int> freeTiles;        // Background tiles the entities go through
	MusicId music;                     // Background track of its levels
};

static const WorldInfo KITCHEN = { "images/kitchen.png", "images/kitchen_box_break.png", { 3, 1 }, { 5, 2 }, { 6, 2 }, { 3 }, MUSIC_KITCHEN };
static const WorldInfo FRIDGE = { "images/fridge.png", "images/fridge_box_break.png", { 6, 0 }, { 6, 4 }, { 6, 5 }, { 3, 4, 5, 11, 12, 13, 19, 20, 21 }, MUSIC_FRIDGE };
// The tilesheet of the oven has no door (door and openDoor are empty cells), and it uses
// the music of the fridge until it has its own track
static const WorldInfo OVEN = { "images/oven.png", "images/oven_box_break.png", { 3, 3 }, { 15, 15 }, { 15, 15 },
	{ 80, 81, 82, 83, 84, 85, 86, 96, 97, 98, 99, 100, 101, 112, 113, 114, 115, 116, 117,
	  39, 40, 41, 55, 56, 57, 71, 72, 73, 87, 88, 89, 103, 104, 105, 119, 120, 121 }, MUSIC_FRIDGE };


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
	{ "levels/2-2", &FRIDGE },
	{ "levels/3-1", &OVEN },
};

static const int NUM_LEVELS = sizeof(LEVELS) / sizeof(LEVELS[0]);


#endif // _LEVELS_INCLUDE
