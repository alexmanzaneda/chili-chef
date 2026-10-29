#include "PowerUp.h"


#define POWER_UP_FRAMES 2 // Columns of powerups.png: the icon and the icon shining
#define SHINE_FPS 4       // It alternates between both frames


PowerUp::PowerUp()
{
	type = POWER_UP_BOMB_UP;
	sprite = NULL;
}

PowerUp::~PowerUp()
{
	if (sprite != NULL)
		delete sprite;
}

void PowerUp::init(PowerUpType type, const glm::ivec2 &tile, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram)
{
	float stepX = 1.0f / POWER_UP_FRAMES;
	float stepY = 1.0f / NUM_POWER_UPS;

	this->type = type;
	this->tile = tile;
	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, SHINE_FPS);
	for (int i = 0; i < POWER_UP_FRAMES; i++)
		sprite->addKeyframe(0, glm::vec2(float(i) * stepX, float(type) * stepY));
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(tile * tileSize));
}

void PowerUp::update(int deltaTime)
{
	sprite->update(deltaTime);
}

void PowerUp::render()
{
	sprite->render();
}

Sprite *PowerUp::createSprite(PowerUpType type, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram)
{
	float stepX = 1.0f / POWER_UP_FRAMES;
	float stepY = 1.0f / NUM_POWER_UPS;

	Sprite *sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, SHINE_FPS); // Sprite::update needs a speed even with one keyframe
	sprite->addKeyframe(0, glm::vec2(stepX, float(type) * stepY)); // The shining frame
	sprite->changeAnimation(0);
	return sprite;
}
