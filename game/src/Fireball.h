#ifndef _FIREBALL_INCLUDE
#define _FIREBALL_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Fireball is a shot of the boss. It flies straight through the platforms, until it reaches
// the walls around the level. GameScene checks if it burns the chef and removes it.


class Fireball
{

public:
	Fireball();
	~Fireball();

	// The spritesheet is shared by all the fireballs and owned by GameScene
	void init(const glm::vec2 &center, const glm::vec2 &direction, TileMap *map, Texture &spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	bool hasHitWall() const { return hitWall; }
	glm::vec2 getCenter() const { return center; }
	// Does it touch a box with that corner and size?
	bool hits(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const;

private:
	Sprite *sprite;
	TileMap *map;
	glm::vec2 center, velocity;
	bool hitWall;

};


#endif // _FIREBALL_INCLUDE
