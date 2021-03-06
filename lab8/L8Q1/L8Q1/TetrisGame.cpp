#include <iostream>
#include <assert.h>
#include "TetrisGame.h"


// constructor
	//   assign pointers,
	//   load font from file: fonts/RedOctober.ttf
	//   setup scoreText
	//   reset the game
TetrisGame::TetrisGame(sf::RenderWindow* pWindow, sf::Sprite* pBlockSprite, Point gameboardOffset, Point nextShapeOffset) {
	assert(TetrisGame::pWindow = pWindow);
	assert(TetrisGame::pBlockSprite = pBlockSprite);
	TetrisGame::gameboardOffset = gameboardOffset;
	TetrisGame::nextShapeOffset = nextShapeOffset;
	reset();

	// Set up font for drawing the score
	if (!scoreFont.loadFromFile("fonts/RedOctober.ttf")) {
		assert(false && "Missing font: RedOctober.ttf");
	}
	scoreText.setFont(scoreFont);
	scoreText.setCharacterSize(24);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition(435, 325);
	draw();
};


// destructor, set pointers to null
TetrisGame::~TetrisGame() {
	TetrisGame::pWindow = nullptr;
	TetrisGame::pBlockSprite = nullptr;
};

// draw anything to do with the game,
// includes board, currentShape, nextShape, score
void TetrisGame::draw() {
	drawGameboard();
	drawTetromino(currentShape, gameboardOffset);
	drawTetromino(nextShape, nextShapeOffset);
	updateScoreDisplay();
	pWindow->draw(scoreText);
};

// Event and game loop processing
// handles keypress events (up, left, right, down, space)
void TetrisGame::onKeyPressed(sf::Event event) {
	switch (event.key.code) {
	case sf::Keyboard::Up:
		attemptRotate(currentShape);
		break;
	case sf::Keyboard::Left:
		attemptMove(currentShape, -1, 0);
		break;
	case sf::Keyboard::Right:
		attemptMove(currentShape, 1, 0);
		break;
	case sf::Keyboard::Down:
		if (!attemptMove(currentShape, 0, 1)) {
			lock(currentShape);
			shapePlacedSinceLastGameLoop = true;
		}
		break;
	case sf::Keyboard::Space:
		drop(currentShape);
		lock(currentShape);
		shapePlacedSinceLastGameLoop = true;
		break;
	}
};

// called every game loop to handle ticks & tetromino placement (locking)
void TetrisGame::processGameLoop(float secondsSinceLastLoop) {
	secondsSinceLastTick += secondsSinceLastLoop;
	if (secondsSinceLastTick > secsPerTick) {
		tick();
		secondsSinceLastTick -= secsPerTick;
	}
	if (shapePlacedSinceLastGameLoop) {
		if (spawnNextShape()) {
			pickNextShape();
		}
		else
		{
			reset();
		}
	}

	int completedRows = board.removeCompletedRows();
	if (completedRows > 0) {
		score += completedRows;
		updateScoreDisplay();
	}

	shapePlacedSinceLastGameLoop = false;
};

// A tick() forces the currentShape to move (if there were no tick,
// the currentShape would float in position forever). This should
// call attemptMove() on the currentShape.  If not successful, lock() 
// the currentShape (it can move no further), and record the fact that a
// shape was placed (using shapePlacedSinceLastGameLoop)
void TetrisGame::tick() {
	//std::cout << "tick tick" << std::endl;
	if (!attemptMove(currentShape, 0, 1)) {
		lock(currentShape);
		shapePlacedSinceLastGameLoop = true;
	}
};

// reset everything for a new game (use existing functions) 
	//  - setScore to 0
	//  - determineSecondsPerTick(),
	//  - clear the gameboard,
	//  - pick & spawn next shape
	//  - pick next shape again
void TetrisGame::reset() {
	score = 0;
	determineSecsPerTick();
	board.empty();
	pickNextShape();
	spawnNextShape();
	pickNextShape();
};

// assign nextShape.setShape a new random shape  
void TetrisGame::pickNextShape() {
	nextShape.setShape(Tetromino::getRandomShape());
};


// copy the nextShape into the currentShape and set 
//   its loc to be the gameboard's spawn loc.
//	 - return true/false based on isPositionLegal()
bool TetrisGame::spawnNextShape() {
	currentShape = nextShape;
	currentShape.setGridLoc(board.getSpawnLoc());
	if (isPositionLegal(currentShape)) {
		return true;
	}
	else {
		return false;
	}
};



// test if a rotation is legal on the tetromino, 
//   if so, rotate it.
//  To do this:
//	 1) create a (local) temporary copy of the tetromino
//	 2) rotate it (shape.rotateCW())
//	 3) test if temp rotatio was legal (isPositionLegal()), 
//      if so - rotate the original tetromino.
//	 4) return true/false to indicate successful movement
bool TetrisGame::attemptRotate(GridTetromino& shape) {
	GridTetromino tempShape = shape;
	tempShape.rotateCW();
	if (isPositionLegal(tempShape)) {
		shape.rotateCW();
		return true;
	}
	else {
		return false;
	}
};


// test if a move is legal on the tetromino, if so, move it.
//  To do this:
//	 1) create a (local) temporary copy of the current shape
//	 2) move it (temp.move())
//	 3) test if temp move was legal (isPositionLegal(),
//      if so - move the original.
//	 4) return true/false to indicate successful movement
bool TetrisGame::attemptMove(GridTetromino& shape, int x, int y) {
	GridTetromino tempShape = shape;
	tempShape.move(x, y);
	if (isPositionLegal(tempShape)) {
		shape.move(x, y);
		return true;
	}
	else {
		return false;
	}
};


// drops the tetromino vertically as far as it can 
//   legally go.  Use attemptMove(). This can be done in 1 line.
void TetrisGame::drop(GridTetromino& shape) {
	int i = 0;
	GridTetromino tempShape = shape;
	for (i; i < Gameboard::MAX_Y; i++) {
		tempShape.move(0, 1);
		if (!isPositionLegal(tempShape)) {
			break;
		}
	}
	attemptMove(shape, 0, i);
};

// copy the contents of the tetromino's mapped block locs to the grid.
//	 1) get current blockshape locs via tetromino.getBlockLocsMappedToGrid()
//	 2) iterate on the mapped block locs and copy the contents (color) 
//      of each to the grid (via gameboard.setGridContent()) 
void TetrisGame::lock(const GridTetromino& shape) {
	for (auto& p : shape.getBlockLocsMappedToGrid()) {
		//board.setContent(p.getX(), p.getY(), static_cast<int>(shape.getColor()));
		board.setContent(p, static_cast<int>(shape.getColor()));
	}
};

// Graphics methods ==============================================

// draw a tetris block sprite on the canvas		
// x,y are meant to be block offsets (not pixels), which means you
// will need to multiply them by BLOCK_WIDTH & BLOCK_HEIGHT
//  for help: www.sfml-dev.org/tutorials/2.5/graphics-sprite.php
//	 1) set the block color using pBlockSprite->setTextureRect()
//   2) set the block loc using pBlockSprite->setPosition()   
//	 3) draw the block using pWindow.draw()
// (pointers to window and sprite were passed into the constructor)
void TetrisGame::drawBlock(int x, int y, TetColor color, Point origin) {
	pBlockSprite->setTextureRect(sf::IntRect(BLOCK_WIDTH*static_cast<int>(color), 0, BLOCK_WIDTH, BLOCK_HEIGHT));
	pBlockSprite->setPosition(origin.getX() + x*BLOCK_WIDTH, origin.getY() + y*BLOCK_HEIGHT);
	pWindow->draw(*pBlockSprite);
};

// draw the gameboard blocks on the window
//   iterate through each row & col, use drawBlock() to 
//   draw a block if it it isn't empty.
void TetrisGame::drawGameboard() {
	for (int x = 0; x < Gameboard::MAX_X; x++) {
		for (int y = 0; y < Gameboard::MAX_Y; y++) {
			if (board.getContent(x, y) != Gameboard::EMPTY_BLOCK) {
				drawBlock(x, y, static_cast<TetColor>(board.getContent(x, y)), gameboardOffset);
			}
		}
	}
};

// draw a tetromino on the window
//	 iterate through each mapped loc & drawBlock() for each.
//   the origin determines a 'base point' from which to calculate block offsets
//   If the Tetromino is on the gameboard: use gameboardOffset (otherwise you 
//   can specify another point as the origin - for the nextShape)
void TetrisGame::drawTetromino(GridTetromino tetromino, Point origin) {
	for (auto& p : tetromino.getBlockLocsMappedToGrid()) {
		if (p.getX() == origin.getX() && p.getY() == origin.getY()) {
			drawBlock(p.getX(), p.getY(), tetromino.getColor(), origin);
		}
		else {
			drawBlock(p.getX(), p.getY(), tetromino.getColor(), origin);
			tetromino.setGridLoc(origin);
		}
	}
};

// update the score display
// form a string "score: ##" to display the current score
// user scoreText.setString() to display it.
void TetrisGame::updateScoreDisplay() {
	//std::cout << score << std::endl;
	std::string stringScore = "score ";
	stringScore += std::to_string(score);
	scoreText.setString(stringScore);
};

// State & gameplay/logic methods ================================

// return true if shape is within borders (isShapeWithinBorders())
//	 and does NOT intersect locked blocks (!doesShapeIntersectLockedBlocks())
bool TetrisGame::isPositionLegal(const GridTetromino& shape) {
	if (isShapeWithinBorders(shape) && !doesShapeIntersectLockedBlocks(shape)) {
		return true;
	}

	return false;
};

// return true if the shape is within the left, right,
//	 and lower border of the grid. (false otherwise)
//   All of a shape's blocks must be on the gameboard to be within borders
bool TetrisGame::isShapeWithinBorders(const GridTetromino& shape) {
	for (auto& p : shape.getBlockLocsMappedToGrid()) {
		if (p.getX() >= 0 && p.getX() < Gameboard::MAX_X && p.getY() < Gameboard::MAX_Y) {
			continue;
		}
		else {
			return false;
		}
	}

	return true;
};

// return true if the shape passed in intersects with content on the gameboard.
//   Use Gameboard's areLocsEmpty() for this, and pass it the shape's mapped locs.
bool TetrisGame::doesShapeIntersectLockedBlocks(const GridTetromino& shape) {
	if (board.areLocsEmpty(shape.getBlockLocsMappedToGrid())) {
		return false;
	}
	else {
		return true;
	}
};

// set secsPerTick 
//   - basic: use MAX_SECS_PER_TICK
//   - advanced: base it on score (higher score results in lower secsPerTick)
void TetrisGame::determineSecsPerTick() {
	if (score > 10) {
		secsPerTick -= 0.20;
	}
	else if (score > 15) {
		secsPerTick -= 0.20;
	}
	else if (score > 20) {
		secsPerTick = MIN_SECS_PER_TICK;
	}
};