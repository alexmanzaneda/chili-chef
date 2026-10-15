#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <string>
#include <vector>
#include "UIRenderer.h"


#define MENU_LINE_HEIGHT 12
#define MENU_CURSOR_SPACE 14 // Room on the left of the options for the cursor


// Menu is a vertical list of options navigated with the arrows and confirmed
// with ENTER (or Z). It is not a Scene: scenes own a Menu and decide what
// each option does (main menu, pause...).


class Menu
{

public:
	Menu();

	// Must be called from Scene::init, once OpenGL is ready (loads the cursor)
	void init(const std::vector<std::string> &newOptions);
	void reset(); // Select the first option again
	void setSelected(int option);

	// Returns the index of the confirmed option, or -1 if none was confirmed
	int update(int deltaTime);
	void render(UIRenderer &ui, float x, float y) const;

	// Size of the options plus the cursor, to fit a box around them
	float width() const;
	float height() const;

private:
	std::vector<std::string> options;
	Texture cursor;
	int selected;
	float currentTime;

};


#endif // _MENU_INCLUDE
