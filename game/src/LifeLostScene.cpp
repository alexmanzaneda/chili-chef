#include "LifeLostScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


#define LIFE_LOST_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define LIFE_LOST_MAX_TIME 3000 // ms until the level starts again by itself


LifeLostScene::LifeLostScene(int lives)
{
	this->lives = lives;
	currentTime = 0.f;
}

void LifeLostScene::init()
{
	ui.init();
	UIRenderer::loadImage(livesImage, "images/ui/lives.png");
}

int LifeLostScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(currentTime >= LIFE_LOST_MAX_TIME)
		return SCENE_GAME;
	if(currentTime >= LIFE_LOST_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_GAME;
	return SCENE_LIFE_LOST;
}

void LifeLostScene::render()
{
	bool blink = currentTime >= LIFE_LOST_MIN_TIME && int(currentTime / 500.f) % 2 == 0;

	// The hat and "x N" centered together: 17 px of image, 6 px of gap and the text at x2
	std::string text = "X " + std::to_string(lives);
	float width = 17.f + 6.f + UIRenderer::textWidth(text, 2);
	float x = float(int((UI_WIDTH - width) / 2.f));

	ui.begin();
	ui.fillScreen(UI_COLOR_DARK);
	ui.drawTextCentered("OUCH!", 45.f, UI_COLOR_RED, 2);
	ui.drawTextCentered("YOU LOST A LIFE", 66.f, UI_COLOR_LIGHT);
	ui.drawImage(livesImage, x, 82.f, 17.f, 16.f);
	ui.drawText(text, x + 23.f, 84.f, UI_COLOR_LIGHT, 2);
	if(blink)
		ui.drawTextCentered("PRESS START", 115.f, UI_COLOR_ORANGE);
	ui.end();
}
