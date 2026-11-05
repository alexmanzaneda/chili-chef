#include "HeatBar.h"


#define OFF_TIME 3000     // ms switched off
#define WARNING_TIME 1000 // ms blinking before it goes on
#define ON_TIME 1500      // ms on, burning
#define BLINK_STEP 100    // ms lit / ms off while it warns

// The tube in its tiles of images/oven.png: the glowing core is 13 px wide, 9 px from the top
// of the tiles of the horizontal bar and 10 px from the left of the ones of the vertical bar
#define TUBE_SIZE 13.f
#define HORIZONTAL_TUBE_TOP 9.f
#define VERTICAL_TUBE_LEFT 10.f


HeatBar::HeatBar()
{
	center = glm::vec2(0.f);
	time = 0;
	off = false;
}

HeatBar::~HeatBar()
{
	for (int i = 0; i < int(sprites.size()); i++)
		delete sprites[i];
	for (int i = 0; i < int(tubeSprites.size()); i++)
		delete tubeSprites[i];
}

void HeatBar::init(const std::vector<HeatBarTile> &tiles, int tileSize, int delay, Texture &tilesheet, Texture &tubes, ShaderProgram &shaderProgram)
{
	int columns = tilesheet.width() / tileSize;
	float step = float(tileSize) / tilesheet.width(); // The tilesheets are square
	glm::vec2 minCorner(1e9f), maxCorner(-1e9f);

	for (int i = 0; i < int(tiles.size()); i++) {
		glm::vec2 pos = glm::vec2(tiles[i].cell * tileSize);
		glm::vec2 cell = glm::vec2(float(tiles[i].litTile % columns), float(tiles[i].litTile / columns)) * step;
		// The image of the tubes has the cells of the tilesheet
		Texture *textures[2] = { &tilesheet, &tubes };
		std::vector<Sprite*> *lists[2] = { &sprites, &tubeSprites };
		for (int t = 0; t < 2; t++) {
			Sprite *sprite = Sprite::createSprite(glm::ivec2(tileSize, tileSize), glm::vec2(step, step), textures[t], &shaderProgram);
			sprite->setNumberAnimations(1);
			sprite->addKeyframe(0, cell);
			sprite->changeAnimation(0);
			sprite->setPosition(pos);
			lists[t]->push_back(sprite);
		}

		if (tiles[i].tube == HEAT_TUBE_HORIZONTAL) {
			burnPos.push_back(pos + glm::vec2(0.f, HORIZONTAL_TUBE_TOP));
			burnSize.push_back(glm::vec2(float(tileSize), TUBE_SIZE));
		}
		else if (tiles[i].tube == HEAT_TUBE_VERTICAL) {
			burnPos.push_back(pos + glm::vec2(VERTICAL_TUBE_LEFT, 0.f));
			burnSize.push_back(glm::vec2(TUBE_SIZE, float(tileSize)));
		}
		minCorner = glm::min(minCorner, pos);
		maxCorner = glm::max(maxCorner, pos + glm::vec2(float(tileSize)));
	}
	center = (minCorner + maxCorner) / 2.f;
	time = -delay;
}

HeatBar::Event HeatBar::update(int deltaTime)
{
	Phase before = phase();
	time += deltaTime;
	Phase now = phase();

	if (now == before)
		return NO_EVENT;
	if (now == PHASE_WARNING)
		return EVENT_WARNING;
	if (now == PHASE_ON)
		return EVENT_ON;
	return NO_EVENT;
}

void HeatBar::render()
{
	Phase now = phase();
	if (now == PHASE_ON) {
		for (int i = 0; i < int(sprites.size()); i++)
			sprites[i]->render();
	}
	else if (now == PHASE_WARNING && ((time % cycleTime()) / BLINK_STEP) % 2 == 0) {
		for (int i = 0; i < int(tubeSprites.size()); i++)
			tubeSprites[i]->render();
	}
}

void HeatBar::switchOff()
{
	off = true;
}

bool HeatBar::burns(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const
{
	if (phase() != PHASE_ON)
		return false;
	for (int i = 0; i < int(burnPos.size()); i++) {
		if (boxPos.x < burnPos[i].x + burnSize[i].x && boxPos.x + boxSize.x > burnPos[i].x &&
			boxPos.y < burnPos[i].y + burnSize[i].y && boxPos.y + boxSize.y > burnPos[i].y)
			return true;
	}
	return false;
}

int HeatBar::cycleTime()
{
	return OFF_TIME + WARNING_TIME + ON_TIME;
}

HeatBar::Phase HeatBar::phase() const
{
	if (off || time < 0)
		return PHASE_OFF;
	int t = time % cycleTime();
	if (t < OFF_TIME)
		return PHASE_OFF;
	if (t < OFF_TIME + WARNING_TIME)
		return PHASE_WARNING;
	return PHASE_ON;
}
