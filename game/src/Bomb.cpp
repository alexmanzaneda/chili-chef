#include "Bomb.h"


#define FUSE_TIME 3000     // ms until the bomb explodes
#define BOMB_FRAMES 18     // Frames of chili_bomb.png: 8 + 8 + 2
#define BOMB_COLUMNS 8     // chili_bomb.png is 8 x 3 cells of 32x32
#define BOMB_ROWS 3


Bomb::Bomb()
{
	sprite = NULL;
	map = NULL;
	fuseTime = 0;
	bSolid = false;
	previousTile = 0;
}

Bomb::~Bomb()
{
	if (sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2 &tile, TileMap *tileMap, Texture &spritesheet, ShaderProgram &shaderProgram)
{
	float stepX = 1.0f / BOMB_COLUMNS;
	float stepY = 1.0f / BOMB_ROWS;
	int tileSize = tileMap->getTileSize();

	this->tile = tile;
	map = tileMap;
	sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);

	// The jar fills up during the whole fuse: 18 frames in FUSE_TIME ms (6 fps for 3 s)
	sprite->setAnimationSpeed(0, BOMB_FRAMES * 1000 / FUSE_TIME);
	for (int i = 0; i < BOMB_FRAMES; i++) {
		sprite->addKeyframe(0, glm::vec2(float(i % BOMB_COLUMNS) * stepX, float(i / BOMB_COLUMNS) * stepY));
	}
	sprite->changeAnimation(0);
	sprite->setPosition(glm::vec2(float(tile.x * tileSize), float(tile.y * tileSize)));
}

void Bomb::update(int deltaTime)
{
	fuseTime += deltaTime;
	sprite->update(deltaTime);
}

void Bomb::render()
{
	sprite->render();
}

bool Bomb::hasExploded() const
{
	return fuseTime >= FUSE_TIME;
}

void Bomb::detonate()
{
	fuseTime = FUSE_TIME;
}

void Bomb::makeSolid()
{
	if (bSolid)
		return;
	previousTile = map->getTile(tile.x, tile.y);
	map->setTile(tile.x, tile.y, TILE_BOMB);
	bSolid = true;
}

void Bomb::removeFromMap()
{
	if (!bSolid)
		return;
	map->setTile(tile.x, tile.y, previousTile);
	bSolid = false;
}
