#include "GameOverScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"


#define GAME_OVER_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define GAME_OVER_MAX_TIME 5000 // ms until it goes on by itself


GameOverScene::GameOverScene(int sceneId)
{
	this->sceneId = sceneId;
	currentTime = 0.f;
}

void GameOverScene::init()
{
	ui.init();
	if(sceneId == SCENE_GAME_OVER)
		UIRenderer::loadImage(image, "images/ui/screen_game_over.png");
	Audio::instance().stopMusic();
	Audio::instance().playSound(sceneId == SCENE_VICTORY ? SOUND_VICTORY : SOUND_GAME_OVER);
}

int GameOverScene::update(int deltaTime)
{
	// After GAME OVER the player can still continue; after the victory, the title
	int next = (sceneId == SCENE_GAME_OVER) ? SCENE_CONTINUE : SCENE_MENU;

	currentTime += deltaTime;

	if(currentTime >= GAME_OVER_MAX_TIME)
		return next;
	if(currentTime >= GAME_OVER_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return next;
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
		ui.drawScreenImage(image);
	ui.end();
}
