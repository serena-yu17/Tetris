#include "main.h"
#include <string>
#include <random>
#include <unordered_map>
#include <math.h>
#include <vector>

using namespace sf;
using namespace std;

const int step = 10;		  // size of each brick
const int padding = step;
const int baseMoveInterval = 400;
Time keyInterval, moveInterval, rotateInterval;
bool rotating = 0;
unordered_map<int_fast32_t, int> pow2;


Clock moveClock, keyClock, rotateClock;
unordered_set<RectangleShape*> rects;
RectangleShape* mainRectGrid[28][57];
int_fast32_t globalGrid[57];
vector<RectangleShape> lines;
unsigned long score = 0;
Text txtScore;

//game control
int gameStatus = 0;   //waiting to start -- 0, game on -- 1, game over -- 2 					   		
Point refreshPoint(12, 6);
Point previewPoint(20, 0);
Block preview = Block();
Block running = Block();
int direction = 0;

//random
random_device rd;
mt19937 mrand(rd());
uniform_int_distribution<int> distShape(0, 4);
uniform_int_distribution<int> distColor(50, 205);

int WinMain()
{
	RenderWindow window(VideoMode(280, 570), "Tetris");
	memset(mainRectGrid, 0, sizeof(mainRectGrid));
	keyInterval = Time(milliseconds(70));
	moveInterval = Time(milliseconds(baseMoveInterval));
	rotateInterval = Time(milliseconds(baseMoveInterval));
	drawLines();

	//generate powers of 2
	for (int i = 0; i < 25; i++)
		pow2[1 << i] = i;

	//Score
	Font font;
	if (!font.loadFromFile("arial.ttf"))
		return EXIT_FAILURE;
	txtScore = Text("Score", font, 20);
	txtScore.setPosition(Vector2f(20, step));
	txtScore.setFillColor(Color::Black);

	//Border
	RectangleShape border(Vector2f(260, 500));
	border.setFillColor(Color::Transparent);
	border.setPosition(Vector2f(padding, 60));
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
			if (direction && keyClock.getElapsedTime() > keyInterval) {

				if (!running.move(direction))			//if collides
				{
					eliminate();
					running.set(preview.foreColor, preview.type, refreshPoint);
					preview.clearGraphic();
					preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
				}
				keyClock.restart();
			}
			if (rotating && rotateClock.getElapsedTime() > rotateInterval)
			{
				running.rotate();
				rotateClock.restart();
			}
		}
		window.clear(Color::White);
		window.draw(border);
		window.draw(txtScore);
		for (auto element : lines)
			window.draw(element);
		for (auto element : rects)
			window.draw(*element);
		window.display();
	}
	return 0;
}

void drawLines()
{
	for (int y = 60; y < 570; y += 10)
	{
		RectangleShape line = RectangleShape(Vector2f(260, 1));
		line.setPosition(Vector2f(padding, y));
		line.setFillColor(Color(20, 20, 20, 20));
		lines.push_back(line);
	}
	for (int x = padding; x < 260 + padding; x += 10)
	{
		RectangleShape line = RectangleShape(Vector2f(1, 500));
		line.setPosition(Vector2f(x, 60));
		line.setFillColor(Color(20, 20, 20, 20));
		lines.push_back(line);
	}
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
		break;
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
		break;
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
	int rowsEliminated = 0;
	bool found = 0;
	int first = 0, last = 0;
	int32_t fullRow = (1 << 25) - 1;
	for (int i = 56; i >= 12 && globalGrid[i]; i--)
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
		for (int i = first; i >= last; i--)
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
		for (int i = first; i >= last; i--)
		{
			if (globalGrid[i] == 0)
			{
				int j = i;
				while (globalGrid[j] == 0)
					j--;
				while (j < 57 && (globalGrid[j] & globalGrid[j - 1]) == 0)
				{
					globalGrid[j - 1] = globalGrid[j] & globalGrid[j - 1];
					globalGrid[j] = 0;
					j++;
					found = 1;
					for (int x = 0; x < 25; x++)
					{
						mainRectGrid[x][j]->setPosition(Vector2f(x * 10.0f, (j - 1) * 10.0f));
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

RectangleShape* drawSquare(int x, int y, Color color)
{
	RectangleShape* sq = new RectangleShape(Vector2f(step, step));
	sq->setPosition(x * 10.0f, y * 10.0f);
	sq->setFillColor(color);
	sq->setOutlineColor(Color::Black);
	sq->setOutlineThickness(1.0);
	return sq;
}

// Class functions

Point::Point(int x0, int y0)
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
	for (int y = 0; y < 5; y++)
		for (int x = 0; x < 5; x++)
			if (grid[y] & (1 << x))
			{
				RectangleShape* newRect = drawSquare(x + position.x, y + position.y, foreColor);
				rects.insert(newRect);
				mainRectGrid[x + position.x][y + position.y] = newRect;
			}
}

void Block::clearGraphic()
{
	for (int y = 0; y < 5; y++)
		for (int x = 0; x < 5; x++)
			if (grid[y] & (1 << x) && mainRectGrid[x + position.x][y + position.y])
			{
				rects.erase(mainRectGrid[x + position.x][y + position.y]);
				delete mainRectGrid[x + position.x][y + position.y];
				mainRectGrid[x + position.x][y + position.y] = NULL;
			}
}

void Block::projection()
{
	// Project the local grid to the global grid
	int_fast32_t shiftLine;
	for (int y = 0; y < 5; y++)
	{
		shiftLine = (int_fast32_t)grid[y] << position.x;
		globalGrid[y + position.y] |= shiftLine;
	}
}

bool Block::collision(Point position, char datagrid[5])
{
	for (int i = 4; i >= 0; i--)
		if (datagrid[i] && position.y + i >= 56)
			return 1;
	int_fast32_t shiftLine;
	for (int y = 0; y < 5; y++)
	{
		shiftLine = (int_fast32_t)datagrid[y] << position.x;
		if (shiftLine & globalGrid[y + position.y])
			return 1;
	}
	return 0;
}

void Block::set(sf::Color fColor, int tpe, const Point& pos)
{
	foreColor = fColor;
	position = Point(pos);
	//generate shape
	type = tpe;
	memset(grid, 0, sizeof(grid));
	switch (type)
	{
	case 0:		//bar
		for (int y = 1; y < 5; y++)
			grid[y] = 0b100;
		break;
	case 1:		//square
		grid[2] = grid[3] = 0b1100;
		break;
	case 2:		// L
		grid[1] = grid[2] = 0b100;
		grid[3] = 0b1100;
		break;
	case 3:		// reverse L
		grid[1] = grid[2] = 0b100;
		grid[3] = 0b110;
		break;
	case 4:		//S
		grid[1] = 0b100;
		grid[2] = 0b1100;
		grid[3] = 0b1000;
		break;
	}
	if (collision(position, grid))
	{
		gameover();
		return;
	}
	findBorder(&left, &right, &bot, grid);
	draw();
}

void Block::rotate()
{
	if (type == 1)
		return;
	char newGrid[5] = { 0,0,0,0,0 };
	for (int y = 0; y < 5; y++)
		for (int x = 0; x < 5; x++)
		{
			if (grid[y] & (1 << x))
			{
				int newX = 4 - y;		//pivot is the point (2,2) in grid
				int newY = x;
				newGrid[newY] |= 1 << newX;
			}
		}
	int nleft, nright, nbot;
	findBorder(&nleft, &nright, &nbot, newGrid);
	if (position.x + nleft < 0 || position.x + nright > 26 || position.y + nbot > 56)
		return;
	if (collision(position, newGrid))
		return;
	left = nleft;
	right = nright;
	bot = nbot;
	clearGraphic();
	memcpy(grid, newGrid, 5);
	draw();
}

bool Block::move(int direction)
{
	if (direction == 0)
		return 1;
	int dx, dy;
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
	int newX = position.x + dx;
	int newY = position.y + dy;
	if (newX + left > 0 && newX + right < 27 && newY + bot < 57)
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
	else
		if (newY + bot == 57)
			return 0;
	return 1;
}

void Block::findBorder(int* left, int* right, int* bot, char grid[5])
{
	*bot = 4;
	while (grid[*bot] == 0)
		(*bot)--;
	bool seeking = 1;
	*left = 0;
	while (!search(*left, grid))
		(*left)++;
	*right = 4;
	while (!search(*right, grid))
		(*right)--;
}

bool search(int x, char grid[5])
{
	for (int y = 0; y < 5; y++)
		if (grid[y] & (1 << x))
			return 1;
	return 0;
}
