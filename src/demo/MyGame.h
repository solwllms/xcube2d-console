#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include "../engine/AbstractGame.h"

struct EnemyShip {
	Rectangle2f rect;
	float angle;
	bool isAlive;

	EnemyShip();
};

struct Bullet {
	Rectangle2f rect;
	Vector2i velocity;
	bool isAlive;

	Bullet();
};

const int LEVEL_SIZE = 48;
const int TILE_SIZE = 32;
const int TILE_SIZE_SRC = 64;

const int TILESHEET_X = 4;
const int TILESHEET_Y = 4;

class MyGame : public AbstractGame {
	private:
		SDL_Color magicPink = SDL_Color{ 255, 0, 255, 255 };

		TTF_Font* gameFnt;

		int level[LEVEL_SIZE][LEVEL_SIZE];

		int frame_timer = 0;
		int frame = 0;
		Rect camera;

		Rectangle2f player;
		float targ_angle = 0;
		float angle = 0;
		Vector2f velocity;

		int remainingShips;
		std::vector<std::shared_ptr<EnemyShip>> enemyShips;

		std::vector<std::shared_ptr<Bullet>> bullets;

		void handleKeyEvents();
		void update();
		void render();
		void renderUI();

		void drawTilemap(int x, int y, SDL_Texture* tilemap, int tile, int scroll_offset = 0);

		void setTitle(const std::string&);
		void changeGameWin(const std::string&);

		void fire(const std::string&);
		void spawnShip(const std::string&);
		void generateWorld(const std::string&);

	public:
        MyGame();
		~MyGame();
};

#endif