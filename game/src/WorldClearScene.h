#ifndef _WORLD_CLEAR_SCENE_INCLUDE
#define _WORLD_CLEAR_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


#define NUM_GEMS 3 // One per world


// WorldClearScene is shown after the last level of a world: the gems of the
// worlds cleared so far are in color and the one just won lights up blinking.


class WorldClearScene : public Scene
{

public:
	WorldClearScene(int level); // The level just cleared, the last one of its world

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	int worldsCleared;
	UIRenderer ui;
	Texture background; // With every gem in gray
	Texture gems[NUM_GEMS];
	float currentTime;
	bool gemSoundPlayed;

};


#endif // _WORLD_CLEAR_SCENE_INCLUDE
