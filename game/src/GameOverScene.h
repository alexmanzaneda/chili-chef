#ifndef _GAME_OVER_SCENE_INCLUDE
#define _GAME_OVER_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// GameOverScene is the end of a game: GAME OVER when the chef dies, or the
// victory screen after the last level. It goes back to the main menu after
// a few seconds or when ENTER is pressed.


class GameOverScene : public Scene
{

public:
	GameOverScene(int sceneId); // SCENE_GAME_OVER or SCENE_VICTORY

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	int sceneId;
	UIRenderer ui;
	float currentTime;

};


#endif // _GAME_OVER_SCENE_INCLUDE
