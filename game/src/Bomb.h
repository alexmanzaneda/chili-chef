#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


#define TILE_BOMB -1 // Value written in the TileMap so the bomb is solid (any value except 0 and 4)


// Bomb is placed on a tile and explodes when its fuse runs out. Once the
// chef leaves its tile it becomes solid, so it can be used as a step.


class Bomb
{

public:
	Bomb();
	~Bomb();

	// The spritesheet is shared by all the bombs and owned by GameScene
	void init(const glm::ivec2 &tile, TileMap *tileMap, Texture &spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	glm::ivec2 getTile() const { return tile; }
	bool hasExploded() const;

	bool isSolid() const { return bSolid; }
	void makeSolid();     // Blocks its tile in the TileMap
	void removeFromMap(); // Restores its tile when it explodes

private:
	glm::ivec2 tile;
	int fuseTime;
	bool bSolid;
	int previousTile; // TileMap value before the bomb made it solid
	Sprite *sprite;
	TileMap *map;

};


#endif // _BOMB_INCLUDE
