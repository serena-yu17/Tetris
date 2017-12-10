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
	int left, right, bot, top;
	void findBorder(int* left, int* right, int* bot, int* top, char grid[5]);
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
void test();	// for debug
void finalize();

//constants that define game board size and game speed
const int step = 15;		  // size of each brick
const int padding = step;	  //left padding distance
const int baseMoveInterval = 120;
const int lowerBorder = 35;
const int topBorder = 5;
const int rightBorder = 18;

//Timers
sf::Time keyInterval, moveInterval, rotateInterval;
sf::Clock moveClock, keyClock, rotateClock;

//grids
std::unordered_set<sf::RectangleShape*> rects;		// hash set of Rectangles for efficient random access when drawing the window
sf::RectangleShape* mainRectGrid[28][56];		// this array manages the coordinates of rectangles
int_fast32_t globalGrid[56];				// manages the coordinates of rectangles in a bit set, for collision check
std::vector<sf::RectangleShape> lines;				// background grid lines

											//Score	board
unsigned long score = 0;
sf::Text txtScore;
sf::Text txtGameOver;
sf::Text txtInit;

//game status
int gameStatus = 0;   //waiting to start -- 0, game on -- 1, game over -- 2 					   		
Point refreshPoint(rightBorder / 2 - 3, topBorder + 1);
Point previewPoint(rightBorder - 5, 1);
Block preview = Block();
Block running = Block();
int direction = 0;
bool rotating = 0;


