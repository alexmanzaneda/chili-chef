#include "InfoScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


// Lines must be at most 24 characters to fit the screen

static const char *INSTRUCTIONS_TEXT =
	"GOAL:\n"
	"DEFEAT ALL THE ENEMIES\n"
	"AND REACH THE DOOR\n"
	"BEFORE TIME RUNS OUT.\n"
	"\n"
	"CONTROLS:\n"
	"LEFT/RIGHT  MOVE\n"
	"UP          JUMP\n"
	"SPACE       BOMB\n"
	"ENTER       PAUSE";

static const char *CREDITS_TEXT =
	"PROGRAMMING AND ART:\n"
	"ALEX MANZANEDA\n"
	"SARA VIDAL\n"
	"\n"
	"BASED ON:\n"
	"POCKET BOMBERMAN (1998)\n"
	"\n"
	"VIDEOJOCS - FIB UPC\n"
	"2026/27";


InfoScene::InfoScene(int sceneId)
{
	this->sceneId = sceneId;
	currentTime = 0.f;
	if(sceneId == SCENE_INSTRUCTIONS)
	{
		title = "HOW TO PLAY";
		body = INSTRUCTIONS_TEXT;
	}
	else
	{
		title = "CREDITS";
		body = CREDITS_TEXT;
	}
}

void InfoScene::init()
{
	ui.init();
}

int InfoScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_ESCAPE) ||
	   Game::instance().getKeyDown(GLFW_KEY_X) || Game::instance().getKeyDown(GLFW_KEY_BACKSPACE))
		return SCENE_MENU;
	return sceneId;
}

void InfoScene::render()
{
	ui.begin();
	ui.drawRect(0.f, 0.f, UI_WIDTH, UI_HEIGHT, UI_COLOR_LIGHT);

	ui.drawTextCentered(title, 13.f, UI_COLOR_DARK, 2);
	ui.drawTextCentered(title, 12.f, UI_COLOR_RED, 2);
	ui.drawText(body, 8.f, 36.f, UI_COLOR_DARK);

	if(int(currentTime / 500.f) % 2 == 0)
		ui.drawTextCentered("ENTER: BACK", 146.f, UI_COLOR_ORANGE);
	ui.end();
}
