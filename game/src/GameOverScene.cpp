#include "GameOverScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


#define GAME_OVER_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define GAME_OVER_MAX_TIME 5000 // ms until it goes back to the menu by itself


GameOverScene::GameOverScene()
{
	currentTime = 0.f;
}

void GameOverScene::init()
{
	ui.init();
}

int GameOverScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(currentTime >= GAME_OVER_MAX_TIME)
		return SCENE_MENU;
	if(currentTime >= GAME_OVER_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_MENU;
	return SCENE_GAME_OVER;
}

void GameOverScene::render()
{
	ui.begin();
	ui.drawRect(0.f, 0.f, UI_WIDTH, UI_HEIGHT, UI_COLOR_DARK);

	ui.drawTextCentered("GAME OVER", 65.f, UI_COLOR_RED, 2);
	if(currentTime >= GAME_OVER_MIN_TIME && int(currentTime / 500.f) % 2 == 0)
		ui.drawTextCentered("PRESS START", 100.f, UI_COLOR_LIGHT);
	ui.end();
}
