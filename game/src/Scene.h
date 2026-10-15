#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


// Scene is the abstract base class of every screen of the game (menu, levels...).
// It implements nothing: Game delegates init, update and render to the active scene.


class Scene
{

public:
	virtual ~Scene() {}

	virtual void init() = 0;
	virtual void update(int deltaTime) = 0;
	virtual void render() = 0;

};


#endif // _SCENE_INCLUDE
