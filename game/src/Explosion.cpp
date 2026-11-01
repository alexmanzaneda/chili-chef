#include "Explosion.h"


#define EXPLOSION_COLUMNS 4 // chili_explosion.png: 4 frames x 7 pieces of 32x32
#define EXPLOSION_ROWS 7
#define EXPLOSION_FPS 12

// The fire must touch the middle of the body (16x16 of the 32x32 sprite). It is smaller than
// the box the chef uses to collide (8 px margins), so standing next to the fire is safe
#define HITBOX_OFFSET 8
#define HITBOX_SIZE 16


// Frames of the animation: it grows to its maximum and retracts at once (burst, full, burst, fade)
static const int FRAME_SEQUENCE[] = { 0, 1, 0, 3 };
static const int N_FRAMES = sizeof(FRAME_SEQUENCE) / sizeof(FRAME_SEQUENCE[0]);
static const int EXPLOSION_TIME = N_FRAMES * 1000 / EXPLOSION_FPS; // 333 ms


Explosion::Explosion()
{
	tileSize = 0;
	time = 0;
}

Explosion::~Explosion()
{
	for (int i = 0; i < int(sprites.size()); i++)
		delete sprites[i];
}

void Explosion::init(const std::vector<ExplosionCell> &cells, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram)
{
	float stepX = 1.0f / EXPLOSION_COLUMNS;
	float stepY = 1.0f / EXPLOSION_ROWS;

	this->cells = cells;
	this->tileSize = tileSize;
	for (int i = 0; i < int(cells.size()); i++) {
		Sprite *sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(stepX, stepY), &spritesheet, &shaderProgram);

		// The row of the spritesheet is the piece, the column the frame
		sprite->setNumberAnimations(1);
		sprite->setAnimationSpeed(0, EXPLOSION_FPS);
		for (int f = 0; f < N_FRAMES; f++) {
			sprite->addKeyframe(0, glm::vec2(float(FRAME_SEQUENCE[f]) * stepX, float(cells[i].piece) * stepY));
		}
		sprite->changeAnimation(0);
		sprite->setPosition(glm::vec2(float(cells[i].tile.x * tileSize), float(cells[i].tile.y * tileSize)));
		sprites.push_back(sprite);
	}
}

void Explosion::update(int deltaTime)
{
	time += deltaTime;
	for (int i = 0; i < int(sprites.size()); i++)
		sprites[i]->update(deltaTime);
}

void Explosion::render()
{
	for (int i = 0; i < int(sprites.size()); i++)
		sprites[i]->render();
}

bool Explosion::isFinished() const
{
	return time >= EXPLOSION_TIME;
}

bool Explosion::hits(const glm::vec2 &pos) const
{
	return hits(pos + glm::vec2(HITBOX_OFFSET), glm::vec2(HITBOX_SIZE));
}

bool Explosion::hits(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const
{
	float left = boxPos.x, top = boxPos.y;
	float right = left + boxSize.x, bottom = top + boxSize.y;

	for (int i = 0; i < int(cells.size()); i++) {
		float tileLeft = float(cells[i].tile.x * tileSize), tileTop = float(cells[i].tile.y * tileSize);
		if (left < tileLeft + tileSize && right > tileLeft && top < tileTop + tileSize && bottom > tileTop)
			return true;
	}
	return false;
}
