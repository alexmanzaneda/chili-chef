#include "WorldClearScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Levels.h"
#include "Audio.h"


#define GEM_LIGHT_TIME 800  // ms until the gem of this world starts to light up
#define GEM_BLINK_TIME 100  // ms of each state while it blinks
#define GEM_BLINKS 7        // Changes between gray and color before it stays in color
#define WORLD_CLEAR_MIN_TIME 1500 // ms before ENTER is accepted, so the gem is always seen
#define WORLD_CLEAR_MAX_TIME 4000 // ms until the game goes on by itself

#define CANVAS_TO_UI (UI_WIDTH / 240.f) // The image is 240 px wide


// In the order they are won: kitchen, fridge, oven. Places of the gray gems in screen_world_clear.png
struct GemInfo
{
	const char *file;
	float x, y, width, height;
};

static const GemInfo GEMS[NUM_GEMS] =
{
	{ "images/ui/screen_gem_green.png", 40.f, 182.f, 33.f, 36.f },
	{ "images/ui/screen_gem_blue.png", 107.f, 200.f, 33.f, 36.f },
	{ "images/ui/screen_gem_red.png", 169.f, 182.f, 33.f, 36.f },
};


WorldClearScene::WorldClearScene(int level)
{
	// The world of a level is the number of times the world changes up to it
	worldsCleared = 1;
	for(int i = 1; i < level; i++)
		if(LEVELS[i].world != LEVELS[i - 1].world)
			worldsCleared++;
	currentTime = 0.f;
	gemSoundPlayed = false;
}

void WorldClearScene::init()
{
	ui.init();
	UIRenderer::loadImage(background, "images/ui/screen_world_clear.png");
	for(int i = 0; i < NUM_GEMS; i++)
		UIRenderer::loadImage(gems[i], GEMS[i].file);
}

int WorldClearScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(!gemSoundPlayed && currentTime >= GEM_LIGHT_TIME)
	{
		Audio::instance().playSound(SOUND_POWER_UP);
		gemSoundPlayed = true;
	}
	if(currentTime >= WORLD_CLEAR_MAX_TIME)
		return SCENE_LEVEL_CLEARED;
	if(currentTime >= WORLD_CLEAR_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_LEVEL_CLEARED;
	return SCENE_WORLD_CLEAR;
}

void WorldClearScene::render()
{
	int blink = int((currentTime - GEM_LIGHT_TIME) / GEM_BLINK_TIME);

	ui.begin();
	ui.drawScreenImage(background);
	for(int i = 0; i < worldsCleared; i++)
	{
		// The gems of the previous worlds are already lit
		if(i == worldsCleared - 1 && (currentTime < GEM_LIGHT_TIME || (blink < GEM_BLINKS && blink % 2 != 0)))
			continue;
		ui.drawImage(gems[i], GEMS[i].x * CANVAS_TO_UI, GEMS[i].y * CANVAS_TO_UI - UI_MARGIN_Y,
			GEMS[i].width * CANVAS_TO_UI, GEMS[i].height * CANVAS_TO_UI);
	}
	ui.end();
}
