#ifndef _GAME_OVER_SCENE_INCLUDE
#define _GAME_OVER_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// GameOverScene is the end of a game: GAME OVER when the chef dies, or the
// victory screen after the last level. After a few seconds or when ENTER is
// pressed it goes to the continue screen (GAME OVER) or to the main menu.


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
	Texture image;         // GAME OVER, or the title of the victory (tools/make_screens.py)
	Texture chef;          // Only the victory: the chef celebrating (images/victory.png)
	float currentTime;

};


#endif // _GAME_OVER_SCENE_INCLUDE
