#ifndef _GAME_SCENE_INCLUDE
#define _GAME_SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Scene.h"
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Toaster.h"


// GameScene contains all the entities of a level.
// It is responsible for updating and render them.


class GameScene : public Scene
{

public:
	GameScene();
	~GameScene();

	void init() override;
	void update(int deltaTime) override;
	void render() override;

private:
	void initShaders();
	void initDynamicTiles(); // renamed from dynamicTiles() to avoid name collision with member vector

private:
	TileMap *map;
	Player *player;

	std::vector<Sprite*> dynamicTiles;
	Texture dynamicSpritesheet;

	std::vector<Toaster*> toasters;

	bool chefAlive;

	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	float cameraX, cameraY;
	float cameraWidth, cameraHeight;

};


#endif // _GAME_SCENE_INCLUDE
