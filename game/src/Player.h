#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();
	glm::vec2 getPosition() const { return posPlayer; }

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);

	void die();                  // Starts the death animation
	bool isDying() const { return bDying; }
	bool deathFinished() const;  // The death animation is over (go to game over)

private:
	void updateDeath(int deltaTime);

private:
	bool bJumping;
	bool bDying;
	int deathTime, deathStartY;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY, landTimer;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

};


#endif // _PLAYER_INCLUDE


