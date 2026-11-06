#include "LifeLostScene.h" // Includes GLEW, which must go before GLFW
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Levels.h"
#include "Audio.h"


#define LIFE_LOST_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define LIFE_LOST_MAX_TIME 3000 // ms until the level starts again by itself

#define LOSE_TIME 700       // ms until the number starts to blink before going down
#define BLINK_TIME 100      // ms of each number while it blinks
#define BLINKS 7            // Changes between both numbers before the new one stays

#define CANVAS_TO_UI (UI_WIDTH / 240.f) // The image is 240 px wide
#define HAT_Y 204.f         // Place of the hat in the image, drawn at x2 like the chef
#define HAT_WIDTH 50.f
#define HAT_HEIGHT 44.f

// The "x" and the digits of the counter (tools/title_font.py), centered with the hat
#define TIMES_WIDTH 17.f
#define DIGIT_WIDTH 15.f
#define TEXT_HEIGHT 31.f
#define COUNTER_GAP 10.f    // Between the hat and the "x"
#define TIMES_GAP 6.f       // Between the "x" and the number


LifeLostScene::LifeLostScene(int lives)
{
	this->lives = lives;
	currentTime = 0.f;
	soundPlayed = false;
}

void LifeLostScene::init()
{
	ui.init();
	UIRenderer::loadImage(background, "images/ui/screen_life_lost.png");
	UIRenderer::loadImage(hat, "images/ui/screen_life_hat.png");
	UIRenderer::loadImage(times, "images/ui/screen_life_x.png");
	for(int i = 0; i < 10; i++)
		UIRenderer::loadImage(digits[i], "images/ui/screen_life_" + std::to_string(i) + ".png");
}

int LifeLostScene::update(int deltaTime)
{
	currentTime += deltaTime;

	// The jingle goes with the number going down (the music stopped when the chef died)
	if(!soundPlayed && currentTime >= LOSE_TIME)
	{
		Audio::instance().playSound(SOUND_LIFE_LOST);
		soundPlayed = true;
	}

	if(currentTime >= LIFE_LOST_MAX_TIME)
		return SCENE_GAME;
	if(currentTime >= LIFE_LOST_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_GAME;
	return SCENE_LIFE_LOST;
}

void LifeLostScene::render()
{
	// "x N" with the lives before dying, blinking with the lives left until these stay
	int blink = int((currentTime - LOSE_TIME) / BLINK_TIME);
	bool lostStillShown = currentTime < LOSE_TIME || (blink < BLINKS && blink % 2 != 0);
	float x = (240.f - (HAT_WIDTH + COUNTER_GAP + TIMES_WIDTH + TIMES_GAP + DIGIT_WIDTH)) / 2.f;
	float textY = HAT_Y + (HAT_HEIGHT - TEXT_HEIGHT) / 2.f;

	ui.begin();
	ui.drawBackdrop(*LEVELS[Game::instance().getLevel() - 1].world);
	ui.drawScreenImage(background);
	drawCanvasImage(hat, x, HAT_Y, HAT_WIDTH, HAT_HEIGHT);
	x += HAT_WIDTH + COUNTER_GAP;
	drawCanvasImage(times, x, textY, TIMES_WIDTH, TEXT_HEIGHT);
	x += TIMES_WIDTH + TIMES_GAP;
	drawCanvasImage(digits[lostStillShown ? lives + 1 : lives], x, textY, DIGIT_WIDTH, TEXT_HEIGHT);
	ui.end();
}

void LifeLostScene::drawCanvasImage(const Texture &image, float x, float y, float width, float height)
{
	ui.drawImage(image, x * CANVAS_TO_UI, y * CANVAS_TO_UI - UI_MARGIN_Y, width * CANVAS_TO_UI, height * CANVAS_TO_UI);
}
