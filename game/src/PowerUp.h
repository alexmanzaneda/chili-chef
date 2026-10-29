#ifndef _POWER_UP_INCLUDE
#define _POWER_UP_INCLUDE


#include "Sprite.h"


// Kinds of power-up. The order is the row of each one in powerups.png
// (tools/make_powerups.py). GameScene::applyPowerUp says what each one does
enum PowerUpType { POWER_UP_BOMB_UP, POWER_UP_FIRE_UP, NUM_POWER_UPS };


// PowerUp is an object left by a destroyed box. It waits on its tile until the
// chef picks it up or an explosion burns it. GameScene applies its effect.


class PowerUp
{

public:
	PowerUp();
	~PowerUp();

	// The spritesheet is shared by all the power-ups and owned by GameScene
	void init(PowerUpType type, const glm::ivec2 &tile, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	PowerUpType getType() const { return type; }
	glm::ivec2 getTile() const { return tile; }

	// Still picture of the power-up (for the effect shown when it is picked up)
	static Sprite *createSprite(PowerUpType type, int tileSize, Texture &spritesheet, ShaderProgram &shaderProgram);

private:
	PowerUpType type;
	glm::ivec2 tile;
	Sprite *sprite;

};


#endif // _POWER_UP_INCLUDE
