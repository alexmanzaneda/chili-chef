#ifndef _SPRITE_INCLUDE
#define _SPRITE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"
#include "AnimKeyframes.h"


// This class is derived from code seen earlier in TexturedQuad but it is also
// able to manage animations stored as a spritesheet. 


class Sprite
{

public:
	Sprite(const glm::vec2 &quadSize, const glm::vec2 &sizeInSpritesheet, Texture *spritesheet, ShaderProgram *program);
	~Sprite();

public:
	// Textured quads can only be created inside an OpenGL context
	static Sprite *createSprite(const glm::vec2 &quadSize, const glm::vec2 &sizeInSpritesheet, Texture *spritesheet, ShaderProgram *program);

	void update(int deltaTime);
	void render() const;
	void free();

	void setNumberAnimations(int nAnimations);
	void setAnimationSpeed(int animId, int keyframesPerSec);
	void addKeyframe(int animId, const glm::vec2 &frame);
	void changeAnimation(int animId);
	int animation() const;
	int keyframe() const; // Keyframe of the current animation shown now
	void setAnimationOnce(int animId); // The animation is played once instead of looping
	bool animationFinished() const;    // A one-shot animation reached its last keyframe

	void setPosition(const glm::vec2 &pos);
	void setRotation(float radians); // Around the center of the quad
	void setFlash(bool flash);       // Drawn white (the black outline stays) to show a hit
	void setTint(const glm::vec4 &tint); // Multiplies the texture ((1,1,1,1) draws it as it is)
	glm::vec2 frame() const;         // Texture coordinates of the keyframe shown now

private:
	Texture *texture;
	ShaderProgram *shaderProgram;
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	glm::vec2 position, size;
	float rotation;
	glm::vec4 tint;
	bool bFinished;
	int currentAnimation, currentKeyframe;
	float timeAnimation;
	glm::vec2 texCoordDispl;
	vector<AnimKeyframes> animations;

};


#endif // _SPRITE_INCLUDE

