#include "MyGame.h"

MyGame::MyGame() : AbstractGame(), box(5, 5, 30, 30) {
	gameFnt = ResourceManager::loadFont("res/fonts/arial.ttf", 72);
	gfx->useFont(gameFnt);
	gfx->setVerticalSync(true);

	mySystem->variable("window_title", "Demo game", this, &MyGame::setTitle);

	mySystem->variable("score", 0);
	mySystem->variable("lives", 3);
	mySystem->variable("num_keys", 5);

	mySystem->variable("game_won", false);
	mySystem->variable("game_win_msg", "YOU WON!");

	mySystem->variable("player_color", { 255, 0, 0 });
	mySystem->variable("key_color", { 0, 255, 0 });
	mySystem->variable("score_color", SDL_COLOR_PINK);

    for (int i = 0; i < mySystem->getValue<int>("num_keys"); i++) {
        std::shared_ptr<GameKey> k = std::make_shared<GameKey>();
        k->isAlive = true;
        k->pos = Point2(getRandom(0, 750), getRandom(0, 550));
        gameKeys.push_back(k);
    }

	mySystem->print("Game loaded!", LINETYPE_SUCCESS);
}

MyGame::~MyGame() {

}

void MyGame::setTitle(const std::string& s) {
	gfx.get()->setWindowTitle(s);
}

void MyGame::handleKeyEvents() {
	int speed = 3;

	if (eventSystem->isReleased(Key::CONSOLE)) {
		mySystem->toggle();
	}
	if (mySystem->getIsOpen()) {
		if (eventSystem->isReleased(Key::RETURN)) {
			eventSystem->clearInputString();
			eventSystem->setCursor(0);
			mySystem->submit();
		}

		if (eventSystem->isReleased(Key::UP)) {
			mySystem->historyUp(eventSystem.get());
		}
		if (eventSystem->isReleased(Key::DOWN)) {
			mySystem->historyDown(eventSystem.get());
		}
		if (eventSystem->isReleased(Key::TAB)) {
			mySystem->autocomplete(eventSystem.get());
		}
		if (eventSystem->isReleased(Key::LEFT)) {
			eventSystem->cursorLeft();
		}
		if (eventSystem->isReleased(Key::RIGHT)) {
			eventSystem->cursorRight();
		}

		return; // only do console input
	}

	if (eventSystem->isPressed(Key::W)) {
		velocity.y = -speed;
	}

	if (eventSystem->isPressed(Key::S)) {
		velocity.y = speed;
	}

	if (eventSystem->isPressed(Key::A)) {
		velocity.x = -speed;
	}

	if (eventSystem->isPressed(Key::D)) {
		velocity.x = speed;
	}
}

void MyGame::update() {
	box.x += velocity.x;
	box.y += velocity.y;

	for (auto key : gameKeys) {
		if (key->isAlive && box.contains(key->pos)) {
			mySystem->setValue("score", mySystem->getValue<int>("score") + 200);
			key->isAlive = false;
			mySystem->setValue("num_keys", mySystem->getValue<int>("num_keys") - 1);

			if (mySystem->getValue<int>("num_keys") == 0) {
				mySystem->setValue("game_won", true);
			}
		}
	}

	velocity.x = 0;
    velocity.y = 0;

	mySystem->update(eventSystem.get(), gfx.get());
}

void MyGame::render() {
	gfx->setDrawColor(mySystem->getValue<SDL_Color>("player_color"));
	gfx->drawRect(box);

	gfx->setDrawColor(mySystem->getValue<SDL_Color>("key_color"));
	for (auto key : gameKeys)
        if (key->isAlive)
		    gfx->drawCircle(key->pos, 5);
}

void MyGame::renderUI() {
	gfx->useFont(gameFnt);

	gfx->setDrawColor(mySystem->getValue<SDL_Color>("score_color"));
	std::string scoreStr = mySystem->getValue<std::string>("score");
	gfx->drawText(scoreStr, 780 - scoreStr.length() * 50, 25);

	if (mySystem->getValue<bool>("game_won"))
		gfx->drawText(mySystem->getValue<std::string>("game_win_msg"), 250, 500);

	mySystem->render(gfx.get());
}