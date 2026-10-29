#ifndef _EFFECT_INCLUDE
#define _EFFECT_INCLUDE


#include "Sprite.h"


// Effect is a sprite that is only decoration: fragments of a broken box, smoke...
// It does not collide with anything. It can fly with gravity and spin, and it
// blinks before disappearing. GameScene removes it once it is finished.


class Effect
{

public:
	// Takes ownership of the sprite. With duration <= 0 the effect lasts until the
	// one-shot animation of the sprite finishes
	Effect(Sprite *sprite, const glm::vec2 &pos, int duration);
	~Effect();

	void setVelocity(const glm::vec2 &velocity) { this->velocity = velocity; } // px/s
	void setGravity(float gravity) { this->gravity = gravity; }                // px/s^2
	void setSpin(float spin) { this->spin = spin; }                            // rad/s
	void setBlinkTime(int blinkTime) { this->blinkTime = blinkTime; }          // Last ms it blinks

	void update(int deltaTime);
	void render();
	bool isFinished() const;

private:
	Sprite *sprite;
	glm::vec2 position, velocity;
	float gravity, spin, angle;
	int duration, blinkTime, time;

};


#endif // _EFFECT_INCLUDE
