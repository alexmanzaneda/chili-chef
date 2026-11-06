#include "ContinueScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"


#define CANVAS_TO_UI (UI_WIDTH / 360.f) // The image is 360 px wide

// The arrow points down, centered over the word of each button (px of screen_continue.png)
static const float CURSOR_X[2] = { 73.f, 277.f };
static const float CURSOR_Y = 188.f;
static const float CURSOR_WIDTH = 22.f, CURSOR_HEIGHT = 14.f;


ContinueScene::ContinueScene()
{
	selected = 0;
	currentTime = 0.f;
}

void ContinueScene::init()
{
	ui.init();
	UIRenderer::loadImage(background, "images/ui/screen_continue.png");
	UIRenderer::loadImage(cursor, "images/ui/screen_continue_cursor.png");
}

int ContinueScene::update(int deltaTime)
{
	currentTime += deltaTime;

	// The buttons are side by side, but up and down also change them
	if(Game::instance().getKeyDown(GLFW_KEY_LEFT) || Game::instance().getKeyDown(GLFW_KEY_RIGHT) ||
		Game::instance().getKeyDown(GLFW_KEY_UP) || Game::instance().getKeyDown(GLFW_KEY_DOWN))
	{
		selected = 1 - selected;
		Audio::instance().playSound(SOUND_MENU_MOVE);
	}
	else if(Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_Z))
	{
		Audio::instance().playSound(SOUND_MENU_SELECT);
		return (selected == 0) ? SCENE_GAME : SCENE_MENU;
	}
	return SCENE_CONTINUE;
}

void ContinueScene::render()
{
	// The arrow bobs one pixel of the image up and down, towards the word it points to
	float bob = (int(currentTime / 300.f) % 2 == 0) ? 0.f : 1.f;

	ui.begin();
	ui.drawScreenImage(background);
	ui.drawImage(cursor, CURSOR_X[selected] * CANVAS_TO_UI, (CURSOR_Y + bob) * CANVAS_TO_UI - UI_MARGIN_Y,
		CURSOR_WIDTH * CANVAS_TO_UI, CURSOR_HEIGHT * CANVAS_TO_UI);
	ui.end();
}
