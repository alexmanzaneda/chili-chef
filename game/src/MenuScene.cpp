#include "MenuScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


// Options of the menu, in the order they are shown
enum MenuOption { OPTION_START, OPTION_INSTRUCTIONS, OPTION_CREDITS, OPTION_EXIT };


MenuScene::MenuScene(int previousScene)
{
	this->previousScene = previousScene;
	currentTime = 0.f;
}

void MenuScene::init()
{
	ui.init();
	UIRenderer::loadImage(logo, "images/ui/chili-pepper.png");
	menu.init({ "START", "HOW TO PLAY", "CREDITS", "EXIT" });

	// Coming back from instructions or credits skips the "PRESS START" title
	// and keeps the cursor on the option we came from
	showOptions = (previousScene == SCENE_INSTRUCTIONS || previousScene == SCENE_CREDITS);
	if(previousScene == SCENE_INSTRUCTIONS)
		menu.setSelected(OPTION_INSTRUCTIONS);
	else if(previousScene == SCENE_CREDITS)
		menu.setSelected(OPTION_CREDITS);
}

int MenuScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(!showOptions)
	{
		if(Game::instance().getKeyDown(GLFW_KEY_ENTER))
		{
			showOptions = true;
			menu.reset();
		}
		return SCENE_MENU;
	}

	switch(menu.update(deltaTime))
	{
	case OPTION_START:
		return SCENE_GAME;
	case OPTION_INSTRUCTIONS:
		return SCENE_INSTRUCTIONS;
	case OPTION_CREDITS:
		return SCENE_CREDITS;
	case OPTION_EXIT:
		return SCENE_EXIT;
	}
	return SCENE_MENU;
}

void MenuScene::render()
{
	ui.begin();
	ui.drawRect(0.f, 0.f, UI_WIDTH, UI_HEIGHT, UI_COLOR_LIGHT);

	// Chili at 1:1 so its pixels match the font pixels, then the title with a drop shadow
	ui.drawImage(logo, 64.f, 4.f, 32.f, 32.f);
	ui.drawTextCentered("CHILI CHEF", 39.f, UI_COLOR_DARK, 2);
	ui.drawTextCentered("CHILI CHEF", 38.f, UI_COLOR_RED, 2);

	if(!showOptions)
	{
		// "PRESS START" blinks every half second
		if(int(currentTime / 500.f) % 2 == 0)
			ui.drawTextCentered("PRESS START", 100.f, UI_COLOR_DARK);
	}
	else
	{
		float boxWidth = menu.width() + 20.f, boxHeight = menu.height() + 16.f;
		float boxX = float(int((UI_WIDTH - boxWidth) / 2.f)), boxY = 66.f;

		ui.drawBox(boxX, boxY, boxWidth, boxHeight);
		menu.render(ui, boxX + 10.f, boxY + 8.f);
	}

	ui.drawTextCentered("FIB - UPC 2026", 146.f, UI_COLOR_ORANGE);
	ui.end();
}
