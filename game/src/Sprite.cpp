#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Sprite.h"


Sprite *Sprite::createSprite(const glm::vec2 &quadSize, const glm::vec2 &sizeInSpritesheet, Texture *spritesheet, ShaderProgram *program)
{
	Sprite *quad = new Sprite(quadSize, sizeInSpritesheet, spritesheet, program);

	return quad;
}


Sprite::Sprite(const glm::vec2 &quadSize, const glm::vec2 &sizeInSpritesheet, Texture *spritesheet, ShaderProgram *program)
{
	float vertices[24] = {0.f, 0.f, 0.f, 0.f, 
												quadSize.x, 0.f, sizeInSpritesheet.x, 0.f, 
												quadSize.x, quadSize.y, sizeInSpritesheet.x, sizeInSpritesheet.y, 
												0.f, 0.f, 0.f, 0.f, 
												quadSize.x, quadSize.y, sizeInSpritesheet.x, sizeInSpritesheet.y, 
												0.f, quadSize.y, 0.f, sizeInSpritesheet.y};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
	posLocation = program->bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program->bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
	texture = spritesheet;
	shaderProgram = program;
	currentAnimation = -1;
	position = glm::vec2(0.f);
	size = quadSize;
	rotation = 0.f;
	tint = glm::vec4(1.f);
	bFinished = false;
}

Sprite::~Sprite()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

void Sprite::update(int deltaTime)
{
	if(currentAnimation >= 0)
	{
		const AnimKeyframes &anim = animations[currentAnimation];
		timeAnimation += deltaTime;
		while(!bFinished && timeAnimation > anim.millisecsPerKeyframe)
		{
			timeAnimation -= anim.millisecsPerKeyframe;
			if(anim.once && currentKeyframe == int(anim.keyframeDispl.size()) - 1)
				bFinished = true;
			else
				currentKeyframe = (currentKeyframe + 1) % anim.keyframeDispl.size();
		}
		texCoordDispl = anim.keyframeDispl[currentKeyframe];
	}
}

void Sprite::render() const
{
	glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.f));
	if(rotation != 0.f)
	{
		modelview = glm::translate(modelview, glm::vec3(size.x / 2.f, size.y / 2.f, 0.f));
		modelview = glm::rotate(modelview, rotation, glm::vec3(0.f, 0.f, 1.f));
		modelview = glm::translate(modelview, glm::vec3(-size.x / 2.f, -size.y / 2.f, 0.f));
	}
	shaderProgram->setUniformMatrix4f("modelview", modelview);
	shaderProgram->setUniform2f("texCoordDispl", texCoordDispl.x, texCoordDispl.y);
	bool tinted = (tint != glm::vec4(1.f));
	if(tinted)
		shaderProgram->setUniform4f("color", tint.r, tint.g, tint.b, tint.a);
	glEnable(GL_TEXTURE_2D);
	texture->use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_TEXTURE_2D);
	if(tinted)
		shaderProgram->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
}

void Sprite::free()
{
	glDeleteBuffers(1, &vbo);
}

void Sprite::setNumberAnimations(int nAnimations)
{
	animations.clear();
	animations.resize(nAnimations);
}

void Sprite::setAnimationSpeed(int animId, int keyframesPerSec)
{
	if(animId < int(animations.size()))
		animations[animId].millisecsPerKeyframe = 1000.f / keyframesPerSec;
}

void Sprite::addKeyframe(int animId, const glm::vec2 &displacement)
{
	if(animId < int(animations.size()))
		animations[animId].keyframeDispl.push_back(displacement);
}

void Sprite::changeAnimation(int animId)
{
	if(animId < int(animations.size()))
	{
		currentAnimation = animId;
		currentKeyframe = 0;
		timeAnimation = 0.f;
		bFinished = false;
		texCoordDispl = animations[animId].keyframeDispl[0];
	}
}

int Sprite::animation() const
{
	return currentAnimation;
}

int Sprite::keyframe() const
{
	return currentKeyframe;
}

void Sprite::setAnimationOnce(int animId)
{
	if(animId < int(animations.size()))
		animations[animId].once = true;
}

bool Sprite::animationFinished() const
{
	return bFinished;
}

void Sprite::setPosition(const glm::vec2 &pos)
{
	position = pos;
}

void Sprite::setRotation(float radians)
{
	rotation = radians;
}

void Sprite::setFlash(bool flash)
{
	// The shader multiplies the texture by 'color': a big value saturates every pixel to white
	// except the black ones, so the sprite becomes a white silhouette with its outline
	tint = flash ? glm::vec4(8.f, 8.f, 8.f, 1.f) : glm::vec4(1.f);
}

void Sprite::setTint(const glm::vec4 &tint)
{
	this->tint = tint;
}

glm::vec2 Sprite::frame() const
{
	return texCoordDispl;
}



