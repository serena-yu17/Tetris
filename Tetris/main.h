#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <unordered_set>

typedef unsigned char byte;

struct Point
{
	byte x, y;
	Point(byte x0, byte y0);
	Point(const Point& other);
	Point() {}
};

class Block
{
private:
	Point position;
	char grid[4] = { 0,0,0,0 };
public:
	byte type;
	sf::Color foreColor;
	Block() {}
	void set(sf::Color fColor, byte type, const Point& position);
	bool rotate();
	void projection();
	void draw();
	void clearGraphic();
	bool collision(Point position, char datagrid[4]);
	bool move(byte direction);
};

void procKeyPress(const sf::Event& event);
void procKeyUp(const sf::Event& event);
void eliminate();
void gameover();
sf::RectangleShape* drawSquare(byte x, byte y, sf::Color color);
void init();
