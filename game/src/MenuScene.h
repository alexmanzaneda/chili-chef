#ifndef _MENU_SCENE_INCLUDE
#define _MENU_SCENE_INCLUDE


#include "Scene.h"


// MenuScene is the main menu of the game (not implemented yet).


class MenuScene : public Scene
{

public:
	void init() override;
	void update(int deltaTime) override;
	void render() override;

};


#endif // _MENU_SCENE_INCLUDE
