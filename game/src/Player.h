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

	// Starts the death animation. Burnt by the fire he comes apart like the enemies
	// (images/chef_death.png); hit by an enemy or out of time he jumps off the screen
	void die(bool burnt = false);
	bool isDying() const { return bDying; }
	bool deathFinished() const;  // The death animation is over (go to game over)

	// For a while nothing hurts him and he blinks (he has just lost the 'Heart')
	void protect(int time);
	bool isProtected() const { return protectTime > 0; }

private:
	void updateDeath(int deltaTime);
	int burnRow() const; // Row of chef_death.png with the frame shown now

private:
	bool bJumping;
	bool bDying, bBurnt;
	int deathTime, deathStartY;
	int protectTime; // ms left without getting hurt
	int godBlinkTime; // ms in god mode, for the blink
	Texture burnSpritesheet;
	Sprite *burnSprite; // Only while he comes apart
	ShaderProgram *program;
	glm::ivec2 tileMapDispl, posPlayer;
	int jumpAngle, startY, landTimer;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

};


#endif // _PLAYER_INCLUDE


