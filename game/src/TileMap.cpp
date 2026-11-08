#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "TileMap.h"


using namespace std;


TileMap* TileMap::createTileMap(const string& levelFile, const glm::vec2& minCoords, ShaderProgram& program)
{
	TileMap* map = new TileMap(levelFile, minCoords, program);

	return map;
}


TileMap::TileMap(const string& levelFile, const glm::vec2& minCoords, ShaderProgram& program)
{
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if (map != NULL)
		delete[] map;
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}


void TileMap::render() const
{
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
}

void TileMap::free()
{
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string& levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	int tile;

	fin.open(levelFile.c_str());
	if (!fin.is_open())
		return false;
	getline(fin, line);
	if (line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);

	map = new int[mapSize.x * mapSize.y];
	for (int j = 0; j < mapSize.y; j++)
	{
		for (int i = 0; i < mapSize.x; i++)
		{
			fin >> tile;
			map[j * mapSize.x + i] = tile + 1;
		}
	}

	fin.close();

	return true;
}

void TileMap::prepareArrays(const glm::vec2& minCoords, ShaderProgram& program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;

	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for (int j = 0; j < mapSize.y; j++)
	{
		for (int i = 0; i < mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			if (tile != 0)
			{
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile - 1) % tilesheetSize.x) / tilesheetSize.x, float((tile - 1) / tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4 * sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2& pos, const glm::ivec2& size, int marginX, int marginY) const
{
	int x, y0, y1;

	// Ya no declaramos marginX ni marginY aquí porque nos llegan por parámetro

	x = (pos.x + marginX) / tileSize;
	y0 = (pos.y + marginY) / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;

	for (int y = y0; y <= y1; y++)
	{
		int tileId = map[y * mapSize.x + x];
		if (!isFree(tileId))
			return true;
	}
	return false;
}

// HAZ LO MISMO CON collisionMoveRight y collisionMoveUp: 
// Pon los parámetros en el título y borra las declaraciones de dentro.

bool TileMap::collisionMoveDown(const glm::ivec2& pos, const glm::ivec2& size, int* posY, int marginX) const
{
	int x0, x1, y;

	x0 = (pos.x + marginX) / tileSize;
	x1 = (pos.x + size.x - 1 - marginX) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;

	for (int x = x0; x <= x1; x++)
	{
		int tileId = map[y * mapSize.x + x];
		if (!isFree(tileId))
		{
			if (*posY - tileSize * y + size.y <= 16)
			{
				*posY = tileSize * y - size.y;
				return true;
			}
		}
	}
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2& pos, const glm::ivec2& size, int marginX, int marginY) const
{
	int x, y0, y1;

	// Restamos marginX para que la caja termine antes de llegar a la pared
	x = (pos.x + size.x - 1 - marginX) / tileSize;

	// Sumamos marginY para ignorar el espacio vacío encima de la cabeza
	y0 = (pos.y + marginY) / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;

	for (int y = y0; y <= y1; y++)
	{
		int tileId = map[y * mapSize.x + x];
		if (!isFree(tileId))
			return true;
	}
	return false;
}

bool TileMap::collisionMoveUp(const glm::ivec2& pos, const glm::ivec2& size, int* posY, int marginX, int marginY) const
{
	int x0, x1, y;

	// Aplicamos el margen horizontal a ambos lados (empezar más a la derecha, terminar más a la izquierda)
	x0 = (pos.x + marginX) / tileSize;
	x1 = (pos.x + size.x - 1 - marginX) / tileSize;

	// Calculamos la altura de la cabeza real del chef (sumando marginY)
	y = (pos.y + marginY) / tileSize;

	// Evitamos que el juego colapse si saltas por encima del límite superior del mapa
	if (y < 0) return false;

	for (int x = x0; x <= x1; x++)
	{
		int tileId = map[y * mapSize.x + x];
		if (!isFree(tileId))
		{
			// Si chocamos, colocamos al chef compensando el espacio transparente de su sombrero
			*posY = (y + 1) * tileSize - marginY;
			return true;
		}
	}
	return false;
}

bool TileMap::isFree(int tileId) const
{
	// The map stores the id of Tiled + 1, so 0 is an empty cell
	return tileId == 0 || std::find(freeTiles.begin(), freeTiles.end(), tileId - 1) != freeTiles.end();
}

int TileMap::getTile(int x, int y) const {
	// Protección para no leer fuera de la memoria
	if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y) return 0;
	return map[y * mapSize.x + x];
}

void TileMap::setTile(int x, int y, int value) {
	if (x >= 0 && x < mapSize.x && y >= 0 && y < mapSize.y) {
		map[y * mapSize.x + x] = value;
	}
}



























