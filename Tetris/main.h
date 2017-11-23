#pragma once
#include <SFML/Graphics.hpp>
#include <utility>
#include <unordered_set>

struct Point
{
	int x, y;
	Point(int x0, int y0);
	Point(const Point& other);
	Point() {}
};

class Block
{
private:
	Point position;
	char grid[5] = { 0,0,0,0,0 };
	int left, right, bot;
	void findBorder(int* left, int* right, int* bot, char grid[5]);
public:
	int type;
	sf::Color foreColor;
	Block() {}
	void set(sf::Color fColor, int type, const Point& position);
	void rotate();
	void projection();
	void draw();
	void clearGraphic();
	bool collision(Point position, char datagrid[5]);
	bool move(int direction);	
};

void procKeyPress(const sf::Event& event);
void procKeyUp(const sf::Event& event);
void eliminate();
void gameover();
sf::RectangleShape* drawSquare(int x, int y, sf::Color color);
void init();
bool search(int x, char grid[5]);
void drawLines();

