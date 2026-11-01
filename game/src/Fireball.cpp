#include <cmath>
#include "Fireball.h"


#define FIREBALL_SIZE 16
#define FIREBALL_FRAMES 4  // images/fireball.png, flying to the right
#define FIREBALL_FPS 12
#define FIREBALL_SPEED 130.f // px/s (the chef runs at 120)
#define HIT_RADIUS 4.f     // Half of the box that burns: only its bright middle


Fireball::Fireball()
{
	sprite = NULL;
	map = NULL;
	hitWall = false;
	leftWall = false;
}

Fireball::~Fireball()
{
	if (sprite != NULL)
		delete sprite;
}

void Fireball::init(const glm::vec2 &center, const glm::vec2 &direction, TileMap *map, Texture &spritesheet, ShaderProgram &shaderProgram)
{
	float step = 1.f / FIREBALL_FRAMES;

	this->map = map;
	this->center = center;
	velocity = direction * FIREBALL_SPEED;

	sprite = Sprite::createSprite(glm::ivec2(FIREBALL_SIZE, FIREBALL_SIZE), glm::vec2(step, 1.f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->setAnimationSpeed(0, FIREBALL_FPS);
	for (int i = 0; i < FIREBALL_FRAMES; i++)
		sprite->addKeyframe(0, glm::vec2(float(i) * step, 0.f));
	sprite->changeAnimation(0);
	sprite->setRotation(std::atan2(direction.y, direction.x));
	sprite->setPosition(center - glm::vec2(FIREBALL_SIZE / 2.f));
}

void Fireball::update(int deltaTime)
{
	// A long frame (window dragged...) must not make it go through a wall
	center += velocity * glm::min(deltaTime / 1000.f, 1.f / 30.f);
	sprite->update(deltaTime);
	sprite->setPosition(center - glm::vec2(FIREBALL_SIZE / 2.f));

	int tileSize = map->getTileSize();
	glm::ivec2 tile = glm::ivec2(center) / tileSize;
	glm::ivec2 mapSize = map->getMapSize();
	if (center.x < 0.f || center.y < 0.f || tile.x >= mapSize.x || tile.y >= mapSize.y) {
		hitWall = true;
		return;
	}

	// The boss floats over the walls: the wall where it is born must not stop it
	bool freeTile = map->isFree(map->getTile(tile.x, tile.y));
	if (freeTile)
		leftWall = true;
	hitWall = !freeTile && leftWall;
}

void Fireball::render()
{
	sprite->render();
}

bool Fireball::hits(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const
{
	return center.x + HIT_RADIUS > boxPos.x && center.x - HIT_RADIUS < boxPos.x + boxSize.x &&
		center.y + HIT_RADIUS > boxPos.y && center.y - HIT_RADIUS < boxPos.y + boxSize.y;
}
