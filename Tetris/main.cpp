#include <string>
#include <random>
#include <unordered_map>
#include <math.h>
#include <vector>

#include "main.h"
#include "font.h"
#include "icon.h"

using namespace sf;
using namespace std;

//random
std::random_device rd;
std::mt19937 mrand(rd());
std::uniform_int_distribution<int> distShape(0, 5);				//random shapes -- 6 basic shapes to start with
std::uniform_int_distribution<int> distColor(50, 205);			//random color space
std::uniform_int_distribution<int> distRotation(0, 3);			//random rotation status

int WinMain()
{
	RenderWindow window(VideoMode((rightBorder + 2) * step, (lowerBorder + 2) * step), "Tetris", Style::Close);
	window.setIcon(icon.width, icon.height, icon.pixel_data);
	window.setVerticalSyncEnabled(1);

	memset(mainRectGrid, 0, sizeof(mainRectGrid));

	keyInterval = Time(milliseconds(baseMoveInterval / 2.5));			//Interval between key-forces moves
	moveInterval = Time(milliseconds(baseMoveInterval));				//Inverval between the regular descent
	rotateInterval = Time(milliseconds(baseMoveInterval * 1.5));		//Interval between rotations

	//prepare grid lines
	drawLines();

	//Text font is supplied in font.h
	Font font;
	if (!font.loadFromMemory(fontChar, fontChar_size))
		return EXIT_FAILURE;

	//score board
	txtScore = Text("Score", font, 20);
	txtScore.setPosition(Vector2f(20, step));
	txtScore.setFillColor(Color::Black);

	//game over text
	txtGameOver = Text("Game Over\n\nRestart: Esc", font, 25);
	txtGameOver.setPosition(Vector2f(70, (lowerBorder - topBorder) / 2 * step));
	txtGameOver.setFillColor(Color::Red);
	txtGameOver.setOutlineColor(Color::Yellow);
	txtGameOver.setOutlineThickness(10);

	//Initial text prompt
	txtInit = Text("Press arrow keys\n        to start", font, 25);
	txtInit.setPosition(Vector2f(40, (lowerBorder - topBorder) / 2 * step));
	txtInit.setFillColor(Color::Black);
	txtInit.setOutlineColor(Color::White);
	txtInit.setOutlineThickness(10);

	//Border
	RectangleShape border(Vector2f(rightBorder * step, (lowerBorder - topBorder) * step));
	border.setFillColor(Color::Transparent);
	border.setPosition(Vector2f(padding, (topBorder + 1) *step));
	border.setOutlineColor(Color::Black);
	border.setOutlineThickness(2.0);

	//event loop
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();			//SFML clears the rectangles automatically, so no need to delete
			else if (event.type == Event::KeyPressed)
				procKeyPress(event);	// key events are split into key-down and key-up events to keep a smooth control
			else if (event.type == Event::KeyReleased)
				procKeyUp(event);
		}

		//when game is on, process the game loop
		if (gameStatus == 1)
		{
			// the regular descent of blocks
			if (moveClock.getElapsedTime() > moveInterval)
			{
				if (!running.move(2))	   //if collides, check if there are rows that can be eliminated
					finalize();
				moveClock.restart();
			}

			//forced move upon arrow keys
			if (direction && keyClock.getElapsedTime() > keyInterval) {

				if (!running.move(direction))			//if collides, check if there are rows that can be eliminated
					finalize();
				keyClock.restart();
			}

			//rotation upon up key
			if (rotating && rotateClock.getElapsedTime() > rotateInterval)
			{
				running.rotate();
				rotateClock.restart();
			}
		}
		window.clear(Color(240, 240, 240, 255));			  //background color
		window.draw(border);
		window.draw(txtScore);
		for (auto element : lines)							  //draw grid lines
			window.draw(element);
		for (auto element : rects)							  //draw the tetris blocks
			window.draw(*element);
		if (gameStatus == 3)								  //if game over, display the "game over" text
			window.draw(txtGameOver);
		else if (gameStatus == 0)							  //If waiting to start, display the initial text
			window.draw(txtInit);
		window.display();
	}
	return 0;
}

// processes the block when it collides or reaches the bottom
void finalize()
{
	rotating = 0;
	direction = 0;
	eliminate();
	running.set(preview.foreColor, preview.type, refreshPoint);
	int rotations = distRotation(mrand);		//set the block at a random rotation
	for (int i = 0; i < rotations; i++)
		running.rotate();
	preview.clearGraphic();
	preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
}

//generates the static grid lines
void drawLines()
{
	for (int y = (topBorder + 1) * step; y < (lowerBorder + 1) * step; y += step)
	{
		RectangleShape line = RectangleShape(Vector2f(rightBorder * step, 1));		  //rectangles of height 1 are essentially "lines"
		line.setPosition(Vector2f(padding, y));
		line.setFillColor(Color(255, 255, 255, 255));
		lines.push_back(line);
	}
	for (int x = padding; x < rightBorder * step + padding; x += step)
	{
		RectangleShape line = RectangleShape(Vector2f(1, (lowerBorder - topBorder) * step));
		line.setPosition(Vector2f(x, (topBorder + 1) * step));
		line.setFillColor(Color(255, 255, 255, 255));
		lines.push_back(line);
	}
}

//key movement handlers
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

//game starter, clears up everything for a fresh new game
void init()
{
	direction = 0;
	rotating = 0;
	gameStatus = 1;
	memset(globalGrid, 0, sizeof(globalGrid));
	for (auto element : rects)
		delete element;
	rects.clear();
	memset(mainRectGrid, 0, sizeof(mainRectGrid));
	score = 0;
	txtScore.setString("0");
	running.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), refreshPoint);
	int rots = distRotation(mrand);
	for (int i = 0; i < rots; i++)
		running.rotate();
	preview.clearGraphic();
	preview.set(Color(distColor(mrand), distColor(mrand), distColor(mrand), 255), distShape(mrand), previewPoint);
	moveInterval = Time(milliseconds(baseMoveInterval));
	//test();
}

//eliminate full rows
void eliminate()
{
	running.projection();					  //project the local grid of the block onto the global grid. Then the local grid will be reused by set()
	int rowsEliminated = 0;					  //counter for rows eliminated
	bool found = 0;
	int high = 0, low;
	int_fast32_t fullRow = (1 << rightBorder) - 1;		   // a full row is filled by 1
	for (low = lowerBorder; low >= topBorder && globalGrid[low]; low--)
	{
		if (high < low && fullRow == globalGrid[low])		 //high: the first full row, low: the last row that is being used before it is blank
			high = low;
	}
	if (high)								   //if no rows are found, leave
		found = 1;
	else
		return;
	while (found)
	{
		found = 0;

		//eliminate full rows
		for (int i = high; i >= low; i--)
			if (globalGrid[i] == fullRow)
			{
				found = 1;
				globalGrid[i] = 0;
				rowsEliminated++;
				for (int x = 0; x <= rightBorder; x++)
				{
					rects.erase(mainRectGrid[x][i]);
					delete mainRectGrid[x][i];
					mainRectGrid[x][i] = NULL;
				}
			}

		//move the above rows down
		for (int i = high; i >= low; i--)
		{
			if (globalGrid[i] == 0)
			{
				int j = i - 1;
				while (j >= low && globalGrid[j] == 0)
					j--;
				while (j < lowerBorder && globalGrid[j] && (globalGrid[j] & globalGrid[j + 1]) == 0)
				{
					globalGrid[j + 1] = globalGrid[j] | globalGrid[j + 1];
					globalGrid[j] = 0;
					found = 1;
					for (int x = 0; x <= rightBorder; x++)
					{
						if (mainRectGrid[x][j])
						{
							mainRectGrid[x][j]->setPosition(Vector2f(x * step + padding, (j + 1) * step));
							mainRectGrid[x][j + 1] = mainRectGrid[x][j];
							mainRectGrid[x][j] = NULL;
						}
					}
					j++;
				}
				i = j;
			}
		}
		//loop to eliminate again until no full row can be found
	}

	//add to the score
	if (rowsEliminated)
	{
		score += rowsEliminated * (rowsEliminated + 1) / 2;
		txtScore.setString(to_string(score * 100));

		//increase game speed based on the score
		int newInterval = baseMoveInterval / (1 + log10(score + 1));
		moveInterval = Time(milliseconds(newInterval));
	}
}

//for debug
void test()
{
	for (int x = 0; x < rightBorder; x++)
	{
		RectangleShape* sq = drawSquare(x, lowerBorder, Color::Black);
		rects.insert(sq);
		mainRectGrid[x][lowerBorder] = sq;
	}
	globalGrid[lowerBorder] = (1 << rightBorder) - 1;
}

void gameover()
{
	gameStatus = 3;
}

//generates the rectangle
RectangleShape* drawSquare(int x, int y, Color color)
{
	RectangleShape* sq = new RectangleShape(Vector2f(step - 1, step - 1));
	sq->setPosition(x * step + padding - 1, y * step - 1);
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

//adds the current block to the graphics grid
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

//clears the current block from the graphics grid
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

//merges the current block into the global grid
void Block::projection()
{
	// Project the local grid to the global grid
	int_fast32_t shiftLine;
	for (int y = 0; y < 5; y++)
	{
		if (grid[y])
		{
			if (position.x >= 0)
				shiftLine = (int_fast32_t)grid[y] << position.x;
			else
				shiftLine = (int_fast32_t)grid[y] >> (-position.x);
			globalGrid[y + position.y] |= shiftLine;
		}
	}
}

//collision test
bool Block::collision(Point position, char datagrid[5])
{
	if (position.y + bot > lowerBorder)
		return 1;
	int_fast32_t shiftLine;
	for (int y = 0; y < 5; y++)
	{
		if (position.x >= 0)
			shiftLine = (int_fast32_t)grid[y] << position.x;
		else
			shiftLine = (int_fast32_t)grid[y] >> (-position.x);
		if (shiftLine & globalGrid[y + position.y])
			return 1;
	}
	return 0;
}

//set new values and positions for the block
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
	case 5:	   //Z
		grid[1] = 0b1000;
		grid[2] = 0b1100;
		grid[3] = 0b100;
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

//rotate the current block by clockwise 90 deg each time
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
	if (position.x + nleft < 0 || position.x + nright >= rightBorder || position.y + nbot > lowerBorder)
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
	if (collision(Point(newX, newY), grid))
		if (direction == 2)
			return 0;
		else
			return 1;
	if (newX + left >= 0 && newX + right < rightBorder && newY + bot <= lowerBorder)
	{
		clearGraphic();
		position.x = newX;
		position.y = newY;
		draw();
	}
	else
		if (newY + bot == lowerBorder + 1)
			return 0;
	return 1;
}

//find the furthest suqares of the current block. Used for movement control to make sure the block remains within the borders
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
