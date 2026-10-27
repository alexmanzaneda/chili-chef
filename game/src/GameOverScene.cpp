#include "GameOverScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


#define GAME_OVER_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define GAME_OVER_MAX_TIME 5000 // ms until it goes back to the menu by itself


GameOverScene::GameOverScene(int sceneId)
{
	this->sceneId = sceneId;
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
	return sceneId;
}

void GameOverScene::render()
{
	bool blink = currentTime >= GAME_OVER_MIN_TIME && int(currentTime / 500.f) % 2 == 0;

	ui.begin();
	if(sceneId == SCENE_VICTORY)
	{
		ui.fillScreen(UI_COLOR_LIGHT);
		ui.drawTextCentered("YOU WIN!", 56.f, UI_COLOR_DARK, 2);
		ui.drawTextCentered("YOU WIN!", 55.f, UI_COLOR_RED, 2);
		ui.drawTextCentered("THANKS FOR PLAYING", 80.f, UI_COLOR_DARK);
		if(blink)
			ui.drawTextCentered("PRESS START", 110.f, UI_COLOR_ORANGE);
	}
	else
	{
		ui.fillScreen(UI_COLOR_DARK);
		ui.drawTextCentered("GAME OVER", 65.f, UI_COLOR_RED, 2);
		if(blink)
			ui.drawTextCentered("PRESS START", 100.f, UI_COLOR_LIGHT);
	}
	ui.end();
}
