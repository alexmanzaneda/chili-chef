#ifndef _GAME_SCENE_INCLUDE
#define _GAME_SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Scene.h"
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Toaster.h"
#include "Bomb.h"
#include "Explosion.h"
#include "UIRenderer.h"
#include "Menu.h"


#define TILE_BOX 12      // Destructible box in the dynamic tiles
#define TILE_DOOR 39     // Exit door in the dynamic tiles
#define MAX_FIRE_RANGE 4 // Maximum explosion range with 'Fire Up' (Pocket Bomberman)


// Box or door placed from the dynamic level file
struct DynamicTile
{
	Sprite *sprite;
	glm::ivec2 tile;
	int id; // Value written in the TileMap (TILE_BOX, door...)
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
	void initDynamicTiles(); // renamed from dynamicTiles() to avoid name collision with member vector
	int updatePause(int deltaTime);
	void renderPause();
	void updateBombs(int deltaTime);
	bool playerOverlapsTile(const glm::ivec2 &tile) const;
	Bomb *bombAt(const glm::ivec2 &tile) const;
	void explodeBomb(Bomb *bomb);
	void destroyBox(const glm::ivec2 &tile);
	void updateExplosions(int deltaTime);
	void killAllEnemies();
	bool enemiesLeft() const;
	void openDoor();
	bool playerAtDoor() const;
	void renderLevelCleared();
	void renderExitMessage();

private:
	int level;
	TileMap *map;
	Player *player;

	// The door opens when all the enemies are dead; walking into it clears the level
	glm::ivec2 doorTile;
	bool doorOpen;
	int doorOpenTime; // ms since the door opened (for the "EXIT!" message)
	bool levelCleared;
	int clearTime;

	std::vector<DynamicTile> dynamicTiles;
	Texture dynamicSpritesheet;

	std::vector<Toaster*> toasters;

	std::vector<Bomb*> bombs;
	Texture bombSpritesheet; // Shared by all the bombs
	int maxBombs;            // Simultaneous bombs ('Bomb Up' increases it)

	std::vector<Explosion*> explosions;
	Texture explosionSpritesheet; // Shared by all the explosions
	int fireRange;                // Tiles the fire reaches in each direction ('Fire Up' increases it)

	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	float cameraX, cameraY;
	float cameraWidth, cameraHeight;

	// Pause is an overlay: the level keeps its state while the menu is shown
	bool paused;
	Menu pauseMenu;
	UIRenderer ui;

};


#endif // _GAME_SCENE_INCLUDE
