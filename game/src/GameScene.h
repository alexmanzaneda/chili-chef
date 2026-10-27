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


// Values of the dynamic layer (levels/level<N>-dynamic.txt, art/maps/objects.tsx in Tiled).
// They are the same in every world, so the levels do not depend on the tileset of each world
enum LevelObject { OBJ_NONE, OBJ_BOX, OBJ_DOOR, OBJ_PLAYER, OBJ_TOASTER };

// Written in the TileMap where a box or the closed door are, so they collide like walls.
// No real tile uses these values, so the fire never confuses a tile of the map with a box
#define TILE_BOX 1000
#define TILE_DOOR 1001
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
	void loadLevelObjects(); // Boxes, door, chef and enemies from the dynamic layer
	void addDynamicTile(const glm::ivec2 &tile, int id);
	void spawnToaster(const glm::ivec2 &tile);
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
	glm::ivec2 playerSpawn; // Tile where the chef appears

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
