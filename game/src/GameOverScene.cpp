#include "GameOverScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"
#include "Levels.h"


#define GAME_OVER_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define GAME_OVER_MAX_TIME 5000 // ms until it goes on by itself
#define VICTORY_MAX_TIME 10000  // The victory stays longer: the chef celebrates

// The chef celebrating (images/victory.png): he raises his arms (frames 0-2) and then keeps
// bouncing with them up (3-7, in a loop)
#define VICTORY_FRAMES 8
#define VICTORY_LOOP_FIRST 3
#define VICTORY_FRAME_TIME 120 // ms
#define VICTORY_FRAME_WIDTH 66  // px of each frame, with room over the hat for the bounce
#define VICTORY_FRAME_HEIGHT 100
#define VICTORY_FEET 98         // Row of the frames under the feet of the chef
#define VICTORY_PIXEL 4.f       // Pixels of the window per pixel of the chef
#define VICTORY_Y 6.f           // UI units: the top of the frames, empty over the hat


GameOverScene::GameOverScene(int sceneId)
{
	this->sceneId = sceneId;
	currentTime = 0.f;
}

void GameOverScene::init()
{
	ui.init();
	if(sceneId == SCENE_GAME_OVER)
		UIRenderer::loadImage(image, "images/ui/screen_game_over.png");
	else
	{
		UIRenderer::loadImage(image, "images/ui/screen_title_you_win.png");
		UIRenderer::loadImage(chef, "images/victory.png");
	}
	Audio::instance().stopMusic();
	Audio::instance().playSound(sceneId == SCENE_VICTORY ? SOUND_VICTORY : SOUND_GAME_OVER);
}

int GameOverScene::update(int deltaTime)
{
	// After GAME OVER the player can still continue; after the victory, the title
	int next = (sceneId == SCENE_GAME_OVER) ? SCENE_CONTINUE : SCENE_MENU;

	currentTime += deltaTime;

	if(currentTime >= (sceneId == SCENE_GAME_OVER ? GAME_OVER_MAX_TIME : VICTORY_MAX_TIME))
		return next;
	if(currentTime >= GAME_OVER_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return next;
	return sceneId;
}

void GameOverScene::render()
{
	bool blink = currentTime >= GAME_OVER_MIN_TIME && int(currentTime / 500.f) % 2 == 0;

	ui.begin();
	if(sceneId == SCENE_VICTORY)
	{
		int frame = int(currentTime / VICTORY_FRAME_TIME);
		if(frame >= VICTORY_FRAMES)
			frame = VICTORY_LOOP_FIRST + (frame - VICTORY_FRAMES) % (VICTORY_FRAMES - VICTORY_LOOP_FIRST);
		float pixel = VICTORY_PIXEL * UI_PIXEL;
		float width = VICTORY_FRAME_WIDTH * pixel, height = VICTORY_FRAME_HEIGHT * pixel;
		float feetY = VICTORY_Y + VICTORY_FEET * pixel;

		ui.drawPaper();
		ui.drawTitle(image, 6.f);
		// A soft shadow on the floor, which stays while he bounces
		ui.drawRect(UI_WIDTH / 2.f - 14.f, feetY - 1.f, 28.f, 2.f, glm::vec4(0.f, 0.f, 0.f, 0.12f));
		ui.drawRect(UI_WIDTH / 2.f - 10.f, feetY - 1.5f, 20.f, 3.f, glm::vec4(0.f, 0.f, 0.f, 0.12f));
		ui.drawImageRegion(chef, glm::vec4(float(frame) / VICTORY_FRAMES, 0.f, float(frame + 1) / VICTORY_FRAMES, 1.f),
			(UI_WIDTH - width) / 2.f, VICTORY_Y, width, height);
		ui.drawTextOutlinedCentered("THANKS FOR PLAYING", 118.f, UI_COLOR_WHITE);
		if(blink)
			ui.drawTextOutlinedCentered("PRESS START", 138.f, UI_COLOR_YELLOW);
	}
	else
	{
		// The floor of the world where the chef lost his last life
		ui.drawBackdrop(*LEVELS[Game::instance().getLevel() - 1].world);
		ui.drawScreenImage(image);
	}
	ui.end();
}
