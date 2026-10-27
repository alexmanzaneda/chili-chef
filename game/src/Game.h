#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define NUM_LEVELS 1 // levels/level1.txt ... levels/level<NUM_LEVELS>.txt


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() : scene(NULL), currentScene(-1), currentLevel(1) {}
	~Game() { delete scene; }
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;     // Is the key held down? (movement)
	bool getKeyDown(int key) const; // Was the key pressed this frame? (menus)

private:
	void changeScene(int sceneId);

private:
	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that
							    // we can have access at any time
	bool keysDown[GLFW_KEY_LAST+1]; // Keys pressed since the last update
	Scene *scene; // Active scene, Game delegates to it
	int currentScene; // SceneId of the active scene
	int currentLevel; // Level loaded by GameScene (1..NUM_LEVELS)

};


#endif // _GAME_INCLUDE


