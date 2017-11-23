#include "main.h"
#include <string>
#include <random>
#include <unordered_map>
#include <math.h>

using namespace sf;
using namespace std;

const byte step = 10;		  // size of each brick
const byte padding = step;
const int baseMoveInterval = 500;
Time keyInterval, moveInterval;
bool rotating = 0;
unordered_map<int_fast32_t, byte> pow2;


Clock moveClock, keyClock;
unordered_set<RectangleShape*> rects;
RectangleShape* mainRectGrid[25][56];
int_fast32_t globalGrid[56];
unsigned long score = 0;
Text txtScore;

//game control
byte gameStatus = 0;   //waiting to start -- 0, game on -- 1, game over -- 2 					   		
Point refreshPoint(12, 6);
Point previewPoint(20, 1);
Block preview = Block();
Block running = Block();
byte direction = 0;

//random
random_device rd;
mt19937 mrand(rd());
uniform_int_distribution<int> distShape(0, 4);
uniform_int_distribution<int> distColor(50, 205);

int WinMain()
{
	RenderWindow window(VideoMode(270, 570), "Tetris");
	memset(mainRectGrid, 0, sizeof(mainRectGrid));
	keyInterval = Time(milliseconds(100));
	moveInterval = Time(milliseconds(baseMoveInterval));

	//generate powers of 2
	for (byte i = 0; i < 25; i++)
		pow2[1 << i] = i;

	//Score
	Font font;
	if (!font.loadFromFile("arial.ttf"))
		return EXIT_FAILURE;
	txtScore = Text("Score", font, 20);
	txtScore.setPosition(Vector2f(step, step));
	txtScore.setFillColor(Color::Black);

	//Border
	RectangleShape border(Vector2f(250, 500));
	border.setFillColor(Color::Transparent);
	border.setPosition(Vector2f(padding, 59));
	border.setOutlineColor(Color::Black);
	border.setOutlineThickness(2.0);

	//event loop
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
			else if (event.type == Event::KeyPressed)
				procKeyPress(event);
			else if (event.type == Event::KeyReleased)
				procKeyUp(event);
		}
		//regular move
		if (gameStatus == 1)
		{
			if (moveClock.getElapsedTime() > moveInterval)
			{
				if (!running.move(2))	   //if collides
				{
					eliminate();
					running.set(preview.foreColor, preview.type, refreshPoint);
					preview.clearGraphic();
					preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
				}
				moveClock.restart();
			}
			//forced move
			if (direction && keyClock.getElapsedTime() > keyInterval)
			{
				if (!running.move(direction))			//if collides
				{
					eliminate();
					running.set(preview.foreColor, preview.type, refreshPoint);
					preview.clearGraphic();
					preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
				}
				keyClock.restart();
			}
			if (rotating && keyClock.getElapsedTime() > keyInterval)
			{
				running.rotate();
				keyClock.restart();
			}
		}
		window.clear(Color::White);
		window.draw(border);
		window.draw(txtScore);
		for (auto element : rects)
			window.draw(*element);
		window.display();
	}
	return 0;
}

void procKeyPress(const Event& event)
{
	switch (event.key.code)
	{
	case Keyboard::Escape:
		init();
		break;
	case Keyboard::Left:
		if (gameStatus == 0)
		{
			gameStatus = 1;
			init();
		}
		direction = 3;
		break;
	case Keyboard::Right:
		if (gameStatus == 0)
		{
			gameStatus = 1;
			init();
		}
		direction = 1;
		break;
	case Keyboard::Down:
		if (gameStatus == 0)
		{
			gameStatus = 1;
			init();
		}
		direction = 2;
		break;
	case Keyboard::Up:
		if (gameStatus == 1)
			rotating = 1;
		else if (gameStatus == 0)
		{
			gameStatus = 1;
			init();
		}
	}
}

void procKeyUp(const Event& event)
{
	switch (event.key.code)
	{
	case Keyboard::Left:
		if (gameStatus == 1 && direction == 3)
			direction = 0;
		break;
	case Keyboard::Right:
		if (gameStatus == 1 && direction == 1)
			direction = 0;
		break;
	case Keyboard::Down:
		if (gameStatus == 1 && direction == 2)
			direction = 0;
		break;
	case Keyboard::Up:
		if (rotating)
			rotating = 0;
	}
}

void init()
{
	direction = 0;
	gameStatus = 1;
	for (auto element : rects)
		delete element;
	rects.clear();
	memset(mainRectGrid, 0, sizeof(mainRectGrid));
	score = 0;
	txtScore.setString("0");
	running.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), refreshPoint);
	preview.clearGraphic();
	preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
}

void eliminate()
{
	running.projection();
	byte rowsEliminated = 0;
	bool found = 0;
	byte first = 0, last = 0;
	int32_t fullRow = (1 << 25) - 1;
	for (byte i = 55; i >= 12 && globalGrid[i]; i--)
	{
		if (!first && globalGrid[i] == fullRow)
			first = i;
		if (!last && globalGrid[i] == 0)
			last = i;
	}
	if (first)
		found = 1;
	else
		return;
	while (found)
	{
		found = 0;
		for (byte i = first; i >= last; i--)
			if (globalGrid[i] == fullRow)
			{
				found = 1;
				globalGrid[i] = 0;
				rowsEliminated++;
				for (int x = 0; x < 25; x++)
				{
					rects.erase(mainRectGrid[x][i]);
					delete mainRectGrid[x][i];
					mainRectGrid[x][i] = NULL;
				}
			}
		for (byte i = first; i >= last; i--)
		{
			if (globalGrid[i] == 0)
			{
				byte j = i;
				while (globalGrid[j] == 0)
					j--;
				while (j < 56 && (globalGrid[j] & globalGrid[j - 1]) == 0)
				{
					globalGrid[j - 1] = globalGrid[j] & globalGrid[j - 1];
					globalGrid[j] = 0;
					j++;
					found = 1;
					for (int x = 0; x < 25; x++)
					{
						mainRectGrid[x][j]->setPosition(Vector2f(x, j - 1));
						mainRectGrid[x][j - 1] = mainRectGrid[x][j];
						mainRectGrid[x][j] = NULL;
					}
				}
				i = j;
			}
		}
	}
	score += (1 << (rowsEliminated - 1)) * 100;
	txtScore.setString(to_string(score));
	moveInterval = Time(milliseconds((double)baseMoveInterval / log2(score)));
}

void gameover()
{
	gameStatus = 3;
}

RectangleShape* drawSquare(byte x, byte y, Color color)
{
	RectangleShape* sq = new RectangleShape(Vector2f(step, step));
	sq->setPosition(x * 10.0f + padding, y * 10.0f);
	sq->setFillColor(color);
	sq->setOutlineColor(Color::Black);
	sq->setOutlineThickness(1.0);
	return sq;
}

// Class functions

Point::Point(byte x0, byte y0)
{
	x = x0;
	y = y0;
}

Point::Point(const Point& other)
{
	x = other.x;
	y = other.y;
}

void Block::draw()
{
	int_fast32_t shiftLine;
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			if (grid[y] & (1 << x))
			{
				RectangleShape* newRect = drawSquare(x + position.x, y + position.y, foreColor);
				rects.insert(newRect);
				mainRectGrid[x + position.x][y + position.y] = newRect;
			}
}

void Block::clearGraphic()
{
	int_fast32_t shiftLine;
	for (int y = 0; y < 4; y++)
	{
		shiftLine = (int_fast32_t)grid[y] << position.x;
		for (int x = 0; x < 4; x++)
			if (grid[y] & (1 << x) && mainRectGrid[x + position.x][y + position.y])
			{
				rects.erase(mainRectGrid[x + position.x][y + position.y]);
				delete mainRectGrid[x + position.x][y + position.y];
				mainRectGrid[x + position.x][y + position.y] = NULL;
			}
	}
}

void Block::projection()
{
	// Project the local grid to the global grid
	int_fast32_t shiftLine;
	for (int y = 0; y < 4; y++)
	{
		shiftLine = (int_fast32_t)grid[y] << position.x;
		globalGrid[y + position.y] |= shiftLine;
	}
}

bool Block::collision(Point position, char datagrid[4])
{
	for (int i = 3; i >= 0; i--)
		if (datagrid[i] && position.y + i >= 55)
			return 1;
	int_fast32_t shiftLine;
	for (int y = 0; y < 4; y++)
	{
		shiftLine = (int_fast32_t)datagrid[y] << position.x;
		if (shiftLine & globalGrid[y + position.y])
			return 1;
	}
	return 0;
}

void Block::set(sf::Color fColor, byte tpe, const Point& pos)
{
	foreColor = fColor;
	position = Point(pos);
	//generate shape
	type = tpe;
	memset(grid, 0, sizeof(grid));
	switch (type)
	{
	case 0:		//bar
		for (int y = 0; y < 4; y++)
			grid[y] = 2;
		break;
	case 1:		//square
		grid[1] = grid[2] = 6;
		break;
	case 2:		// L
		grid[0] = grid[1] = 2;
		grid[2] = 6;
		break;
	case 3:		// reverse L
		grid[0] = grid[1] = 2;
		grid[2] = 3;
		break;
	case 4:		//S
		grid[0] = grid[2] = 2;
		grid[1] = 6;
		break;
	}
	if (collision(position, grid))
	{
		gameover();
		return;
	}
	draw();
}

bool Block::rotate()
{
	char newGrid[4] = { 0,0,0,0 };
	for (byte y = 0; y < 4; y++)
		for (byte x = 0; x < 4; x++)
		{
			byte newX = (position.x + 1) - (y - (position.y + 1));		//pivot is the point (1,1) in grid
			byte newY = position.y + 1 + (x - (position.x + 1));
			if (position.x + newX < 0 || position.x + newX >24 || position.y + newY < 0 || position.y + newY >55)
				return 0;
			newGrid[newY] |= (grid[y] & 1 << x) << newX;
		}
	if (collision(position, newGrid))
		return 0;
	clearGraphic();
	memcpy(grid, newGrid, 4);
	draw();
	return 1;
}

bool Block::move(byte direction)
{
	if (direction == 0)
		return 1;
	byte dx, dy;
	switch (direction)
	{
	case 1:		//right
		dx = 1;
		dy = 0;
		break;
	case 2:		//down
		dx = 0;
		dy = 1;
		break;
	case 3:		//left
		dx = -1;
		dy = 0;
		break;
	}
	byte newX = position.x + dx;
	byte newY = position.y + dy;
	if (newX >= 0 && newX < 25 && newY >= 0 && newY < 56)
	{
		if (collision(Point(newX, newY), grid))
			if (direction == 2)
				return 0;
			else
				return 1;
		clearGraphic();
		position.x = newX;
		position.y = newY;
		draw();
	}
	return 1;
}
