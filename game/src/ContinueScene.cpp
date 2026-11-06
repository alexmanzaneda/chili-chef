#include "ContinueScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"


#define CANVAS_TO_UI (UI_WIDTH / 360.f) // The image is 360 px wide

// Center of each button and where its word goes (px of screen_continue.png). The arrow points
// down to the word of the selected one
static const char *LABELS[2] = { "CONTINUE", "QUIT" };
static const float BUTTON_CENTER_X[2] = { 84.f, 288.5f };
static const float LABEL_Y = 207.f;
static const float CURSOR_Y = 188.f;
static const float CURSOR_WIDTH = 22.f, CURSOR_HEIGHT = 14.f;
#define LABEL_SCALE (2.f / 3.f) // Pixels of 3 px of window, the same as the pixels of the image
#define TITLE_Y 18.f            // UI units, over the chef


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
	UIRenderer::loadImage(title, "images/ui/screen_title_continue.png");
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
	ui.drawPaper();
	ui.drawScreenImage(background);
	ui.drawTitle(title, TITLE_Y);
	for(int i = 0; i < 2; i++)
	{
		float width = UIRenderer::textWidth(LABELS[i], LABEL_SCALE);
		ui.drawTextOutlined(LABELS[i], BUTTON_CENTER_X[i] * CANVAS_TO_UI - width / 2.f, LABEL_Y * CANVAS_TO_UI - UI_MARGIN_Y,
			i == selected ? UI_COLOR_YELLOW : UI_COLOR_WHITE, LABEL_SCALE);
	}
	ui.drawImage(cursor, (BUTTON_CENTER_X[selected] - CURSOR_WIDTH / 2.f) * CANVAS_TO_UI, (CURSOR_Y + bob) * CANVAS_TO_UI - UI_MARGIN_Y,
		CURSOR_WIDTH * CANVAS_TO_UI, CURSOR_HEIGHT * CANVAS_TO_UI);
	ui.end();
}
