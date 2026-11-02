#ifndef _LEVEL_INTRO_SCENE_INCLUDE
#define _LEVEL_INTRO_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// LevelIntroScene shows the title screen of a level (Levels.h) before it starts.


class LevelIntroScene : public Scene
{

public:
	LevelIntroScene(int level); // 1..NUM_LEVELS, with an intro screen

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	int level;
	UIRenderer ui;
	Texture image;
	float currentTime;

};


#endif // _LEVEL_INTRO_SCENE_INCLUDE
