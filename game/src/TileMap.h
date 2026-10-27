#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"


// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.


class TileMap
{

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	// Tile maps can only be created inside an OpenGL context
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();
	int getTile(int x, int y) const;
	void setTile(int x, int y, int value);

	// Tiles of the tilesheet the entities can go through (ids as in Tiled); the rest are solid
	void setFreeTiles(const std::vector<int> &tiles) { freeTiles = tiles; }
	bool isFree(int tileId) const;

	void render() const;
	void free();
	
	int getTileSize() const { return tileSize; }
	glm::ivec2 getMapSize() const { return mapSize; }

	// Añadimos marginX y marginY al final de cada función
	bool collisionMoveLeft(const glm::ivec2& pos, const glm::ivec2& size, int marginX = 0, int marginY = 0) const;
	bool collisionMoveRight(const glm::ivec2& pos, const glm::ivec2& size, int marginX = 0, int marginY = 0) const;
	bool collisionMoveUp(const glm::ivec2& pos, const glm::ivec2& size, int* posY, int marginX = 0, int marginY = 0) const;
	bool collisionMoveDown(const glm::ivec2& pos, const glm::ivec2& size, int* posY, int marginX = 0) const;
	
private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);

private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
	int *map;
	std::vector<int> freeTiles;

};


#endif // _TILE_MAP_INCLUDE


