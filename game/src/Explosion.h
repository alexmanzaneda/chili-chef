#ifndef _EXPLOSION_INCLUDE
#define _EXPLOSION_INCLUDE


#include <vector>
#include "Sprite.h"


// Pieces of the cross, in the same order as the rows of chili_explosion.png
enum ExplosionPiece
{
	PIECE_CENTER, PIECE_HORIZONTAL, PIECE_VERTICAL,
	PIECE_END_LEFT, PIECE_END_RIGHT, PIECE_END_UP, PIECE_END_DOWN
};

struct ExplosionCell
{
	glm::ivec2 tile;
	int piece; // ExplosionPiece
};


// Explosion is the cross of fire left by a bomb. GameScene computes which
// tiles it covers; the explosion animates them and tells what it touches.


class Explosion
{

public:
	Explosion();
	~Explosion();

	// The spritesheet is shared by all the explosions and owned by GameScene
	void init(const std::vector<ExplosionCell> &cells, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	bool isFinished() const;
	// Does the fire touch the middle of the body of an entity at pos?
	bool hits(const glm::vec2 &pos) const;
	// Does the fire touch a box with that corner and size?
	bool hits(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const;

private:
	std::vector<ExplosionCell> cells;
	std::vector<Sprite*> sprites; // One per cell
	int tileSize;
	int time;

};


#endif // _EXPLOSION_INCLUDE
