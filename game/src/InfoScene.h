#ifndef _INFO_SCENE_INCLUDE
#define _INFO_SCENE_INCLUDE


#include <string>
#include "Scene.h"
#include "UIRenderer.h"


// InfoScene shows a static screen of text (instructions or credits) on a panel under its
// title, and goes back to the main menu with ENTER or ESC. The instructions have several
// pages, turned with LEFT/RIGHT: the goal and the controls, and then the power-ups.


class InfoScene : public Scene
{

public:
	InfoScene(int sceneId); // SCENE_INSTRUCTIONS or SCENE_CREDITS

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	void changePage(int newPage);
	void renderText(const std::string &text, float y);
	void renderPowerUps(int powerUpPage); // 0 is the first page of power-ups

private:
	int sceneId;
	std::string body;
	UIRenderer ui;
	Texture title; // tools/make_screens.py
	float currentTime;
	int page, numPages;
	Texture powerUpIcons; // images/powerups.png, only for the instructions

};


#endif // _INFO_SCENE_INCLUDE
