#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


// Identifiers of the scenes. Scene::update returns one of them to tell Game
// which scene must be active in the next frame (its own id to stay).

enum SceneId
{
	SCENE_MENU, SCENE_GAME, SCENE_INSTRUCTIONS, SCENE_CREDITS, SCENE_GAME_OVER,
	SCENE_RESTART, // Recreate the current scene from scratch
	SCENE_EXIT     // Close the game
};


// Scene is the abstract base class of every screen of the game (menu, levels...).
// It implements nothing: Game delegates init, update and render to the active scene.


class Scene
{

public:
	virtual ~Scene() {}

	virtual void init() = 0;
	virtual int update(int deltaTime) = 0;
	virtual void render() = 0;

};


#endif // _SCENE_INCLUDE
