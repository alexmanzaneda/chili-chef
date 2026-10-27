#ifndef _LIFE_LOST_SCENE_INCLUDE
#define _LIFE_LOST_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// LifeLostScene is shown when the chef dies but still has lives: it shows the
// lives left and then the same level starts again from the beginning.


class LifeLostScene : public Scene
{

public:
	LifeLostScene(int lives); // Lives left after losing one

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	int lives;
	UIRenderer ui;
	Texture livesImage; // The hat of the HUD
	float currentTime;

};


#endif // _LIFE_LOST_SCENE_INCLUDE
