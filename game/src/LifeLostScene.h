#ifndef _LIFE_LOST_SCENE_INCLUDE
#define _LIFE_LOST_SCENE_INCLUDE


#include "Scene.h"
#include "UIRenderer.h"


// LifeLostScene is shown when the chef dies but still has lives: a hat per life, and
// the one just lost turns gray blinking (with many lives, a hat and "x N" counting down).
// Then the same level starts again from the beginning.


class LifeLostScene : public Scene
{

public:
	LifeLostScene(int lives); // Lives left after losing one

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	void drawCanvasImage(const Texture &image, float x, float y, float width, float height); // In px of the image

private:
	int lives;
	UIRenderer ui;
	Texture background;                // Edited by hand (screen_life_lost.png)
	Texture hat, grayHat;              // tools/make_screens.py
	Texture times, digits[10];         // "x N" when there are too many lives for a hat each
	float currentTime;

};


#endif // _LIFE_LOST_SCENE_INCLUDE
