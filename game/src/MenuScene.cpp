#include "MenuScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"


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
	UIRenderer::loadImage(background, "images/ui/screen_menu.png");
#ifdef __EMSCRIPTEN__
	// In the browser the game cannot close its tab, so there is no EXIT
	menu.init({ "START", "HOW TO PLAY", "CREDITS" });
#else
	menu.init({ "START", "HOW TO PLAY", "CREDITS", "EXIT" });
#endif
	// Instructions and credits do not change it, so it goes on when we come back from them
	Audio::instance().playMusic(MUSIC_MENU);

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
			Audio::instance().playSound(SOUND_MENU_SELECT);
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
	ui.drawPaper();
	ui.drawScreenImage(background);

	// The box goes in the hole the image leaves on the right of the chef, centered on him.
	// 2 px more on the right than on the left: the outline of the text sticks out on that side
	float boxWidth = menu.width() + 16.f, boxHeight = menu.height() + 14.f;
	float boxX = 58.f, boxY = 66.f;

	if(!showOptions)
	{
		// "PRESS START" alone, without the box (it only frames the options), blinking every half second
		if(int(currentTime / 500.f) % 2 == 0)
			ui.drawTextOutlined("PRESS START", float(int(boxX + (boxWidth - UIRenderer::textWidth("PRESS START")) / 2.f)), boxY + 25.f, UI_COLOR_WHITE);
	}
	else
	{
		ui.drawPanel(boxX, boxY, boxWidth, boxHeight);
		menu.render(ui, boxX + 7.f, boxY + 7.f, UI_COLOR_WHITE, UI_COLOR_YELLOW, true);
	}
	ui.end();
}
