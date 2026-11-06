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
	// Heating bars (only the oven): the tiles of the bars switched off, which the maps use (the
	// lit one is HEAT_BAR_LIT_OFFSET before), and among them the ones the tube crosses
	std::vector<int> heatBars, horizontalTubes, verticalTubes;
	const char *heatBarTubes; // Only the tubes of the lit bars, in the cells of the tilesheet (same tool)
	int sheetColumns, floorTile; // Tiles per row of the tilesheet and the floor behind the screens of the world
	float veil;                  // White over that floor, so the screens stand out on it
};

// tools/make_oven_unlit.py draws each bar switched off 8 rows below the lit one
#define HEAT_BAR_LIT_OFFSET 128

static const WorldInfo KITCHEN = { "images/kitchen.png", "images/kitchen_box_break.png", { 3, 1 }, { 5, 2 }, { 6, 2 }, { 3, 28 }, MUSIC_KITCHEN, {}, {}, {}, NULL, 8, 3, 0.4f };
static const WorldInfo FRIDGE = { "images/fridge.png", "images/fridge_box_break.png", { 6, 0 }, { 6, 4 }, { 6, 5 }, { 3, 4, 5, 7, 11, 12, 13, 19, 20, 21 }, MUSIC_FRIDGE, {}, {}, {}, NULL, 8, 12, 0.25f };
// The tilesheet of the oven has no door (door and openDoor are empty cells): its level
// ends when the boss dies. Its bars: the horizontal one (6x3 tiles) and the vertical one
// (3x6), lit and switched off
static const WorldInfo OVEN = { "images/oven.png", "images/oven_box_break.png", { 3, 3 }, { 15, 15 }, { 15, 15 },
	{ 80, 81, 82, 83, 84, 85, 86, 96, 97, 98, 99, 100, 101, 112, 113, 114, 115, 116, 117,
	  39, 40, 41, 55, 56, 57, 71, 72, 73, 87, 88, 89, 103, 104, 105, 119, 120, 121,
	  208, 209, 210, 211, 212, 213, 224, 225, 226, 227, 228, 229, 240, 241, 242, 243, 244, 245,
	  167, 168, 169, 183, 184, 185, 199, 200, 201, 215, 216, 217, 231, 232, 233, 247, 248, 249 }, MUSIC_OVEN,
	{ 208, 209, 210, 211, 212, 213, 224, 225, 226, 227, 228, 229, 240, 241, 242, 243, 244, 245,
	  167, 168, 169, 183, 184, 185, 199, 200, 201, 215, 216, 217, 231, 232, 233, 247, 248, 249 },
	{ 225, 226, 227, 228 }, { 184, 200, 216, 232 }, "images/oven_tubes.png", 16, 86, 0.15f };


struct LevelInfo
{
	const char *file;        // "levels/1-1" -> levels/1-1.txt + levels/1-1-dynamic.txt
	const WorldInfo *world;
	const char *introScreen; // Image shown before the level starts (tools/make_screens.py), NULL for none
};

static const LevelInfo LEVELS[] =
{
	{ "levels/1-1", &KITCHEN, "images/ui/screen_kitchen_1.png" },
	{ "levels/1-2", &KITCHEN, "images/ui/screen_kitchen_2.png" },
	{ "levels/2-1", &FRIDGE, "images/ui/screen_fridge_1.png" },
	{ "levels/2-2", &FRIDGE, "images/ui/screen_fridge_2.png" },
	{ "levels/3-1", &OVEN, "images/ui/screen_oven.png" },
};

static const int NUM_LEVELS = sizeof(LEVELS) / sizeof(LEVELS[0]);


#endif // _LEVELS_INCLUDE
