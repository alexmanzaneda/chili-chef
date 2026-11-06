#ifndef _GAME_SCENE_INCLUDE
#define _GAME_SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Scene.h"
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Toaster.h"
#include "Pea.h"
#include "Boss.h"
#include "Flame.h"
#include "Fireball.h"
#include "HeatBar.h"
#include "Bomb.h"
#include "Explosion.h"
#include "Effect.h"
#include "PowerUp.h"
#include "UIRenderer.h"
#include "Menu.h"
#include "HUD.h"


// Values of the dynamic layer (levels/level<N>-dynamic.txt, art/maps/objects.tsx in Tiled).
// They are the same in every world, so the levels do not depend on the tileset of each world.
// A power-up of the map is hidden under a box: it appears when the box is destroyed
enum LevelObject { OBJ_NONE, OBJ_BOX, OBJ_DOOR, OBJ_PLAYER, OBJ_TOASTER, OBJ_PEA, OBJ_BOMB_UP, OBJ_FIRE_UP, OBJ_HEART, OBJ_EXTRA_LIFE, OBJ_TIME, OBJ_FULL_FIRE, OBJ_BOSS, OBJ_FLAME };

// Written in the TileMap where a box or the closed door are, so they collide like walls.
// No real tile uses these values, so the fire never confuses a tile of the map with a box
#define TILE_BOX 1000
#define TILE_DOOR 1001
#define TILE_BOX_BURNING 1002 // Box hit by the fire: still solid until the fire is over, but it cannot break again
#define MAX_FIRE_RANGE 4 // Maximum explosion range with 'Fire Up' (Pocket Bomberman)
#define MAX_BOMBS 8      // Maximum simultaneous bombs with 'Bomb Up'

#define NO_POWER_UP -1


// Box or door placed from the dynamic level file
struct DynamicTile
{
	Sprite *sprite;
	glm::ivec2 tile;
	int id;      // Value written in the TileMap (TILE_BOX, door...)
	int powerUp; // PowerUpType the box leaves when it is destroyed, or NO_POWER_UP
};


// GameScene contains all the entities of a level.
// It is responsible for updating and render them.


class GameScene : public Scene
{

public:
	GameScene(int level); // Loads levels/level<level>.txt and levels/level<level>-dynamic.txt
	~GameScene();

	void init() override;
	int update(int deltaTime) override;
	void render() override;

private:
	void initShaders();
	void loadLevelObjects(); // Boxes, door, chef and enemies from the dynamic layer
	void addDynamicTile(const glm::ivec2 &tile, int id, int powerUp = NO_POWER_UP);
	void spawnEnemy(Enemy *enemy, const glm::ivec2 &tile); // Takes ownership of the enemy
	void setPaused(bool pause);
	int updatePause(int deltaTime);
	void renderPause();
	void updateBombs(int deltaTime);
	bool playerOverlapsTile(const glm::ivec2 &tile) const;
	Bomb *bombAt(const glm::ivec2 &tile) const;
	void explodeBomb(Bomb *bomb);
	void destroyBox(const glm::ivec2 &tile);
	void updateBoxes(int deltaTime);
	void spawnPowerUp(PowerUpType type, const glm::ivec2 &tile);
	PowerUp *powerUpAt(const glm::ivec2 &tile) const;
	void removePowerUp(PowerUp *powerUp);
	void updatePowerUps(int deltaTime);
	void applyPowerUp(PowerUpType type);
	void hurtPlayer(bool burnt = false); // The 'Heart' saves him once
	void killPlayer(bool burnt = false); // No 'Heart' can save him (the time ran out, or the 'Heart' is gone)
	void updateExplosions(int deltaTime);
	void updateEffects(int deltaTime);
	void updateDyingEnemies(int deltaTime);
	void updateBoss();                     // Gets what the boss asks for: fireballs, flames, blasts
	void updateFireballs(int deltaTime);
	void clearFireballs();
	void spawnBlast(const glm::vec2 &center); // A round burst of fire that is only decoration (only with a boss)
	void loadHeatBars(); // From the tiles of the map
	void updateHeatBars(int deltaTime);
	void switchOffHeatBars();
	bool isHeatBarTile(int x, int y) const;
	bool onScreen(const glm::vec2 &pos) const;
	void clearLevel();
	void updateCamera(); // Centers it on the chef, without leaving the map
	void killAllEnemies();
	void countEnemies(int count[NUM_ENEMY_TYPES]) const; // Alive enemies of each EnemyType
	bool enemiesLeft() const;
	void openDoor();
	bool playerAtDoor() const;
	void renderExitMessage();

private:
	int level;
	TileMap *map;
	Player *player;
	glm::ivec2 playerSpawn; // Tile where the chef appears

	// The door opens when all the enemies are dead; walking into it clears the level.
	// A level with no door (the boss) is cleared as soon as they are dead
	glm::ivec2 doorTile;
	bool doorOpen;
	int doorOpenTime; // ms since the door opened (for the "EXIT!" message)
	bool levelCleared;

	std::vector<DynamicTile> dynamicTiles;
	Texture dynamicSpritesheet;
	Texture boxBreakSpritesheet; // Frames of the box coming apart (one row of 32x32)

	std::vector<Enemy*> enemies; // NULL once killed
	Boss *boss;                  // Also in enemies; NULL if the level has none or once it is gone

	std::vector<HeatBar*> heatBars; // They use dynamicSpritesheet, the tilesheet of the world
	Texture heatBarTubes;           // And this one, while they warn

	std::vector<Fireball*> fireballs;
	Texture fireballSpritesheet; // Shared by all the fireballs
	Texture burstSpritesheet;    // Round bursts of fire of the boss (spawnBlast)

	std::vector<Bomb*> bombs;
	Texture bombSpritesheet; // Shared by all the bombs
	int maxBombs;            // Simultaneous bombs ('Bomb Up' increases it)

	std::vector<Explosion*> explosions;
	Texture explosionSpritesheet; // Shared by all the explosions
	int fireRange;                // Tiles the fire reaches in each direction ('Fire Up' increases it)
	bool heart;                   // The next hit only takes it away ('Heart' power-up)

	std::vector<PowerUp*> powerUps; // Left by the destroyed boxes, waiting to be picked up
	Texture powerUpSpritesheet;     // Shared by all the power-ups

	std::vector<Effect*> effects; // Decoration only: smoke, sparks...
	int timeLeft;                 // ms of the countdown; the chef dies when it reaches 0

	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	float cameraX, cameraY;
	float cameraWidth, cameraHeight;

	// Pause is an overlay: the level keeps its state while the menu is shown
	bool paused;
	Menu pauseMenu;
	UIRenderer ui;
	HUD hud; // Band at the bottom: countdown, lives, bombs, fire range and enemies left

};


#endif // _GAME_SCENE_INCLUDE
