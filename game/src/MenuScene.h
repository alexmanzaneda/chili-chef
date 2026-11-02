#ifndef _MENU_SCENE_INCLUDE
#define _MENU_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"
#include "Menu.h"


// MenuScene is the main menu: a "PRESS START" title screen followed
// by the list of options.


class MenuScene : public Scene
{

public:
	MenuScene(int previousScene); // SceneId we come from, -1 when the game starts

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	UIRenderer ui;
	Menu menu;
	Texture background; // tools/make_screens.py
	int previousScene;
	bool showOptions; // false while "PRESS START" is shown
	float currentTime;

};


#endif // _MENU_SCENE_INCLUDE
