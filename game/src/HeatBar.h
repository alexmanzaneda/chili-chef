#ifndef _HEAT_BAR_INCLUDE
#define _HEAT_BAR_INCLUDE


#include <vector>
#include "Sprite.h"


// Which part of a tile of a bar burns: the band of the tube that crosses it, if any
enum HeatTube { HEAT_TUBE_NONE, HEAT_TUBE_HORIZONTAL, HEAT_TUBE_VERTICAL };

struct HeatBarTile
{
	glm::ivec2 cell;
	int litTile; // Tiled id of the tile drawn while the bar is on
	int tube;    // HeatTube
};


// HeatBar is one of the heating bars of the oven. The map draws it switched off; it switches
// on every now and then, drawing its lit tiles over the map, and then its tube burns the chef.
// Before, to warn, only its tube blinks hot: the glow (and the floor under it) stays as it is.


class HeatBar
{

public:
	enum Event { NO_EVENT, EVENT_WARNING, EVENT_ON };

	HeatBar();
	~HeatBar();

	// The tilesheet is the one of the world, and tubes an image of its size with only the tubes
	// of the lit tiles; both owned by GameScene. It starts switched off for delay ms, so not all
	// the bars of the level go on at the same time
	void init(const std::vector<HeatBarTile> &tiles, int tileSize, int delay, Texture &tilesheet, Texture &tubes, ShaderProgram &shaderProgram);
	Event update(int deltaTime); // What has started in this update
	void render();

	void switchOff(); // For good: the level is over
	bool burns(const glm::vec2 &boxPos, const glm::vec2 &boxSize) const;
	glm::vec2 getCenter() const { return center; }
	static int cycleTime(); // ms from one time it goes on to the next

private:
	enum Phase { PHASE_OFF, PHASE_WARNING, PHASE_ON };
	Phase phase() const;

private:
	std::vector<Sprite*> sprites;     // One per tile, lit
	std::vector<Sprite*> tubeSprites; // One per tile, only its tube (while it warns)
	std::vector<glm::vec2> burnPos, burnSize; // The parts of the tube, in pixels of the level
	glm::vec2 center;
	int time; // ms in its cycle; negative while it waits to start
	bool off;

};


#endif // _HEAT_BAR_INCLUDE
