#include "LifeLostScene.h" // Includes GLEW, which must go before GLFW
#include <algorithm>
#include <GLFW/glfw3.h>
#include "Game.h"


#define LIFE_LOST_MIN_TIME 1000 // ms before ENTER is accepted, so it is not skipped by accident
#define LIFE_LOST_MAX_TIME 3000 // ms until the level starts again by itself

#define HAT_LOSE_TIME 700   // ms until the hat of the life just lost starts to blink
#define HAT_BLINK_TIME 100  // ms of each state while it blinks
#define HAT_BLINKS 7        // Changes between lit and gray before it stays gray

#define CANVAS_TO_UI (UI_WIDTH / 240.f) // The image is 240 px wide
#define HAT_Y 204.f         // Place of the hats in the image, drawn at x2 like the chef
#define HAT_WIDTH 50.f
#define HAT_HEIGHT 44.f
#define HAT_STEP 56.f       // From one hat to the next
#define MAX_HATS 4          // The most that fit; with more lives (up to MAX_LIVES with 'Extra life') a hat and "x N" are shown

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
}

void LifeLostScene::init()
{
	ui.init();
	UIRenderer::loadImage(background, "images/ui/screen_life_lost.png");
	UIRenderer::loadImage(hat, "images/ui/screen_life_hat.png");
	UIRenderer::loadImage(grayHat, "images/ui/screen_life_hat_gray.png");
	UIRenderer::loadImage(times, "images/ui/screen_life_x.png");
	for(int i = 0; i < 10; i++)
		UIRenderer::loadImage(digits[i], "images/ui/screen_life_" + std::to_string(i) + ".png");
}

int LifeLostScene::update(int deltaTime)
{
	currentTime += deltaTime;

	if(currentTime >= LIFE_LOST_MAX_TIME)
		return SCENE_GAME;
	if(currentTime >= LIFE_LOST_MIN_TIME && Game::instance().getKeyDown(GLFW_KEY_ENTER))
		return SCENE_GAME;
	return SCENE_LIFE_LOST;
}

void LifeLostScene::render()
{
	// The life just lost blinks before it goes: its hat turns gray, or the counter goes down
	int blink = int((currentTime - HAT_LOSE_TIME) / HAT_BLINK_TIME);
	bool lostStillLit = currentTime < HAT_LOSE_TIME || (blink < HAT_BLINKS && blink % 2 != 0);

	ui.begin();
	ui.drawScreenImage(background);
	if(lives + 1 > MAX_HATS)
	{
		float x = (240.f - (HAT_WIDTH + COUNTER_GAP + TIMES_WIDTH + TIMES_GAP + DIGIT_WIDTH)) / 2.f;
		float textY = HAT_Y + (HAT_HEIGHT - TEXT_HEIGHT) / 2.f;

		drawCanvasImage(hat, x, HAT_Y, HAT_WIDTH, HAT_HEIGHT);
		x += HAT_WIDTH + COUNTER_GAP;
		drawCanvasImage(times, x, textY, TIMES_WIDTH, TEXT_HEIGHT);
		x += TIMES_WIDTH + TIMES_GAP;
		drawCanvasImage(digits[lostStillLit ? lives + 1 : lives], x, textY, DIGIT_WIDTH, TEXT_HEIGHT);
	}
	else
	{
		// The lives left are lit and the one just lost (the next hat) turns gray; the rest are gray.
		// There are always START_LIVES hats, more if 'Extra life' gave more
		int hats = std::max(START_LIVES, lives + 1);
		float x = (240.f - (hats * HAT_STEP - (HAT_STEP - HAT_WIDTH))) / 2.f;

		for(int i = 0; i < hats; i++)
		{
			bool lit = i < lives || (i == lives && lostStillLit);
			drawCanvasImage(lit ? hat : grayHat, x + i * HAT_STEP, HAT_Y, HAT_WIDTH, HAT_HEIGHT);
		}
	}
	ui.end();
}

void LifeLostScene::drawCanvasImage(const Texture &image, float x, float y, float width, float height)
{
	ui.drawImage(image, x * CANVAS_TO_UI, y * CANVAS_TO_UI - UI_MARGIN_Y, width * CANVAS_TO_UI, height * CANVAS_TO_UI);
}
