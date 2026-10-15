#ifndef _INFO_SCENE_INCLUDE
#define _INFO_SCENE_INCLUDE


#include <string>
#include "Scene.h"
#include "UIRenderer.h"


// InfoScene shows a static screen of text (instructions or credits)
// and goes back to the main menu when a key is pressed.


class InfoScene : public Scene
{

public:
	InfoScene(int sceneId); // SCENE_INSTRUCTIONS or SCENE_CREDITS

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	int sceneId;
	std::string title, body;
	UIRenderer ui;
	float currentTime;

};


#endif // _INFO_SCENE_INCLUDE
