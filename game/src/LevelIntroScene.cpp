#include "LevelIntroScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Levels.h"
#include "Audio.h"


#define LEVEL_INTRO_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define LEVEL_INTRO_MAX_TIME 3000 // ms until the level starts by itself


LevelIntroScene::LevelIntroScene(int level)
{
	this->level = level;
	currentTime = 0.f;
}

void LevelIntroScene::init()
{
	ui.init();
	UIRenderer::loadImage(image, LEVELS[level - 1].introScreen);
	// Silence until GameScene starts the track of the level from the beginning
	Audio::instance().stopMusic();
}

int LevelIntroScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(currentTime >= LEVEL_INTRO_MAX_TIME)
		return SCENE_GAME;
	if(currentTime >= LEVEL_INTRO_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_GAME;
	return SCENE_LEVEL_INTRO;
}

void LevelIntroScene::render()
{
	ui.begin();
	ui.drawBackdrop(*LEVELS[level - 1].world);
	ui.drawScreenImage(image);
	ui.end();
}
