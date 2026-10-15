#include "Menu.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"


Menu::Menu()
{
	selected = 0;
	currentTime = 0.f;
}

void Menu::init(const std::vector<std::string> &newOptions)
{
	options = newOptions;
	UIRenderer::loadImage(cursor, "images/ui/chili-pepper.png");
	reset();
}

void Menu::reset()
{
	selected = 0;
	currentTime = 0.f;
}

void Menu::setSelected(int option)
{
	if(option >= 0 && option < int(options.size()))
		selected = option;
}

int Menu::update(int deltaTime)
{
	int nOptions = int(options.size());

	currentTime += deltaTime;
	if(nOptions == 0)
		return -1;

	// getKeyDown: one press moves the cursor exactly one option
	if(Game::instance().getKeyDown(GLFW_KEY_UP))
		selected = (selected + nOptions - 1) % nOptions;
	else if(Game::instance().getKeyDown(GLFW_KEY_DOWN))
		selected = (selected + 1) % nOptions;
	else if(Game::instance().getKeyDown(GLFW_KEY_ENTER) || Game::instance().getKeyDown(GLFW_KEY_Z))
		return selected;

	return -1;
}

void Menu::render(UIRenderer &ui, float x, float y) const
{
	for(int i = 0; i < int(options.size()); i++)
	{
		float optionY = y + i * MENU_LINE_HEIGHT;

		if(i == selected)
		{
			// The chili cursor bobs one pixel left and right. The 32x32 image is
			// drawn at half size, centered on the line of text
			float bob = (int(currentTime / 300.f) % 2 == 0) ? 0.f : 1.f;
			ui.drawImage(cursor, x - 3.f + bob, optionY - 5.f, 16.f, 16.f);
			ui.drawText(options[i], x + MENU_CURSOR_SPACE, optionY, UI_COLOR_RED);
		}
		else
			ui.drawText(options[i], x + MENU_CURSOR_SPACE, optionY, UI_COLOR_DARK);
	}
}

float Menu::width() const
{
	float widest = 0.f;

	for(const std::string &option : options)
		if(UIRenderer::textWidth(option) > widest)
			widest = UIRenderer::textWidth(option);
	return MENU_CURSOR_SPACE + widest;
}

float Menu::height() const
{
	if(options.empty())
		return 0.f;
	return float((options.size() - 1) * MENU_LINE_HEIGHT + 7); // 7 = glyph height
}
