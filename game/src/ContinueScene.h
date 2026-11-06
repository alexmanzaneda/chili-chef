#ifndef _CONTINUE_SCENE_INCLUDE
#define _CONTINUE_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// ContinueScene comes after GAME OVER: CONTINUE starts a new game from the first
// level and QUIT goes back to the title screen. The arrows choose and ENTER (or Z) confirms.


class ContinueScene : public Scene
{

public:
	ContinueScene();

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	UIRenderer ui;
	Texture background, cursor; // Edited by hand (screen_continue.png, screen_continue_cursor.png)
	Texture title;              // tools/make_screens.py
	int selected;               // 0 CONTINUE, 1 QUIT
	float currentTime;

};


#endif // _CONTINUE_SCENE_INCLUDE
