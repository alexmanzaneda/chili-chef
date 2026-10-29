#include "Effect.h"


#define BLINK_STEP 50 // ms visible / ms hidden while it blinks


Effect::Effect(Sprite *sprite, const glm::vec2 &pos, int duration)
{
	this->sprite = sprite;
	this->duration = duration;
	position = pos;
	velocity = glm::vec2(0.f);
	gravity = 0.f;
	spin = 0.f;
	angle = 0.f;
	blinkTime = 0;
	time = 0;
	sprite->setPosition(position);
}

Effect::~Effect()
{
	delete sprite;
}

void Effect::update(int deltaTime)
{
	float dt = deltaTime / 1000.f;

	time += deltaTime;
	velocity.y += gravity * dt;
	position += velocity * dt;
	angle += spin * dt;

	sprite->update(deltaTime);
	sprite->setPosition(position);
	sprite->setRotation(angle);
}

void Effect::render()
{
	// The shader discards the half-transparent pixels, so it blinks instead of fading out
	if (duration > 0 && time > duration - blinkTime && (time / BLINK_STEP) % 2 != 0)
		return;
	sprite->render();
}

bool Effect::isFinished() const
{
	if (duration <= 0)
		return sprite->animationFinished();
	return time >= duration;
}
