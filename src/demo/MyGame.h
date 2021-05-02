#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include "../engine/AbstractGame.h"

struct GameKey {
	Point2 pos;
	bool isAlive;
};

class MyGame : public AbstractGame {
	private:
		TTF_Font* gameFnt;
		Rect box;

		Vector2i velocity;

		std::vector<std::shared_ptr<GameKey>> gameKeys;

		void handleKeyEvents();
		void update();
		void render();
		void renderUI();

		void setTitle(const std::string&);
		void sayHi(const std::string&);

	public:
        MyGame();
		~MyGame();
};

#endif