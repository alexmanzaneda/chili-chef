#ifndef _GAME_OVER_SCENE_INCLUDE
#define _GAME_OVER_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// GameOverScene is shown when the chef dies. It goes back to the main menu
// after a few seconds or when ENTER is pressed.


class GameOverScene : public Scene
{

public:
	GameOverScene();

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	UIRenderer ui;
	float currentTime;

};


#endif // _GAME_OVER_SCENE_INCLUDE
