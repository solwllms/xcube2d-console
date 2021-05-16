#include "MyGame.h"

MyGame::MyGame() : AbstractGame(), player(0, 0, 33, 56), camera(0, 0, 0, 0) {
	gameFnt = ResourceManager::loadFont("res/fonts/arial.ttf", 36);
	gfx->useFont(gameFnt);
	gfx->setVerticalSync(true);

	// textures
	ResourceManager::loadTexture("res/textures/tilesheet.png", magicPink);
	ResourceManager::loadTexture("res/textures/water.png", SDL_COLOR_BLACK);
	ResourceManager::loadTexture("res/textures/player.png", magicPink);
	ResourceManager::loadTexture("res/textures/canonball.png", magicPink);
	ResourceManager::loadTexture("res/textures/player.png", magicPink);
	ResourceManager::loadTexture("res/textures/enemy.png", magicPink);
	ResourceManager::loadTexture("res/textures/enemy_dead.png", magicPink);

	// sounds
	ResourceManager::loadSound("res/sounds/fire.wav");
	ResourceManager::loadSound("res/sounds/break.wav");
	ResourceManager::loadSound("res/sounds/win.wav");
	ResourceManager::loadMP3("res/sounds/ambience.mp3");

	// variables
	mySystem->variable("window_title", "Sol Williams - Demo game", this, &MyGame::setTitle);
	mySystem->variable("gui_color", SDL_COLOR_WHITE);

	mySystem->variable("score", 0);
	mySystem->variable("lives", 3);
	mySystem->variable("num_enemies", 5);

	mySystem->variable("game_win", false, this, &MyGame::changeGameWin);
	mySystem->variableStr("game_win_msg", "Game win!");

	mySystem->variable("player_speed", 2);
	mySystem->variable("player_acceleration", 1);
	mySystem->variable("bullet_speed", 10);

	mySystem->variableStr("player_tex", "res/textures/player.png");
	mySystem->variableStr("enemy_tex", "res/textures/enemy.png");

	// functions
	mySystem->function("fire", this, &MyGame::fire);
	mySystem->function("spawnship", this, &MyGame::spawnShip);
	mySystem->function("newworld", this, &MyGame::generateWorld);

	generateWorld("");

	sfx->playMP3(ResourceManager::getMP3("res/sounds/ambience.mp3"), -1);
}

MyGame::~MyGame() {

}

void MyGame::setTitle(const std::string& s) {
	gfx.get()->setWindowTitle(s);
}

void MyGame::changeGameWin(const std::string& s) {
	if(mySystem->getValue<bool>("game_win"))
		sfx->playSound(ResourceManager::getSound("res/sounds/win.wav"));
}

void MyGame::generateWorld(const std::string& s) {

	for (int x = 0; x < LEVEL_SIZE; x++)
	{
		for (int y = 0; y < LEVEL_SIZE; y++)
		{
			level[x][y] = getRandom(0, 8) == 2 ? getRandom(1, 6) : 0;
		}
	}

	enemyShips.clear();
	remainingShips = 0;
	for (int i = 0; i < mySystem->getValue<int>("num_enemies"); i++) {
		spawnShip("");
	}

	player.x = (gfx->getCurrentWindowSize().w - 66) / 2;
	player.y = (gfx->getCurrentWindowSize().h - 113) / 2;

	mySystem->print("World generated!", LINETYPE_SUCCESS);
}


void MyGame::spawnShip(const std::string& s) {

	int x = getRandom(0, (LEVEL_SIZE - 3) * TILE_SIZE);
	int y = getRandom(0, (LEVEL_SIZE - 3) * TILE_SIZE);

	if (s.length() != 0) {
		int space = s.find(' ');
		if (space != std::string::npos) {
			try {
				x = std::stoi(s.substr(0, space));
				y = std::stoi(s.substr(space + 1, s.size() - 1));
			}
			catch (...) {
				mySystem->print("could not parse parameters.", LINETYPE_ERROR);
			}
		}
		else {
			mySystem->print("spawnship [X] [Y]");
			return;
		}
	}

	std::shared_ptr<EnemyShip> k = std::make_shared<EnemyShip>();
	k->isAlive = true;
	k->angle = getRandom(0, M_PI * 2);
	k->rect = Rectangle2f(x, y, 33, 56);
	enemyShips.push_back(k);

	remainingShips++;

	mySystem->print("spawned ship at (x: "+std::to_string(k->rect.x)+", y: "+ std::to_string(k->rect.y) + ")");
}

void MyGame::fire(const std::string& s) {
	int speed = mySystem->getValue<int>("bullet_speed");
	std::shared_ptr<Bullet> k = std::make_shared<Bullet>();
	k->isAlive = true;
	k->rect = Rectangle2f(player.x + ((float)player.w / 2), player.y + ((float)player.h / 2), 16, 16);
	k->velocity = Vector2i(velocity.x + (cos(angle) * -speed), velocity.y + (sin(angle) * -speed));
	bullets.push_back(k);

	sfx->playSound(ResourceManager::getSound("res/sounds/fire.wav"));

	mySystem->print("spawned bullet at (x: " + std::to_string(k->rect.x) + ", y: " + std::to_string(k->rect.y) + ")");
}

void MyGame::handleKeyEvents() {
	float speed = mySystem->getValue<float>("player_speed");
	float acc = mySystem->getValue<float>("player_acceleration") / 10;

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
			mySystem->historyUp(eventSystem);
		}
		if (eventSystem->isReleased(Key::DOWN)) {
			mySystem->historyDown(eventSystem);
		}
		if (eventSystem->isReleased(Key::TAB)) {
			mySystem->autocomplete(eventSystem);
		}
		if (eventSystem->isReleased(Key::LEFT)) {
			eventSystem->cursorLeft();
		}
		if (eventSystem->isReleased(Key::RIGHT)) {
			eventSystem->cursorRight();
		}
	}
	else {

		if (eventSystem->isPressed(Key::W)) {
			velocity.y += -acc;
		}
		if (eventSystem->isPressed(Key::S)) {
			velocity.y += acc;
		}
		if (eventSystem->isPressed(Key::A)) {
			velocity.x += -acc;
		}
		if (eventSystem->isPressed(Key::D)) {
			velocity.x += acc;
		}
		if (eventSystem->isReleased(Key::SPACE)) {
			fire("");
		}
	}

	velocity.x = std::max(-speed, std::min(speed, velocity.x));
	velocity.y = std::max(-speed, std::min(speed, velocity.y));

	//velocity.x = 0;
	if (velocity.x > 0) velocity.x -= 0.05;
	else if (velocity.x < 0)  velocity.x += 0.05;
	if (abs(velocity.x) < 0.05) velocity.x = 0;
	//velocity.x -= 1;

	//velocity.y = 0;
	if (velocity.y > 0) velocity.y -= 0.05;
	else if (velocity.y < 0) velocity.y += 0.05;
	if (abs(velocity.y) < 0.05) velocity.y = 0;
}

float mag(Vector2f a)
{
	return std::sqrt(a.x * a.x + a.y * a.y);
}

float dot(Vector2f a, Vector2f b)
{
	return a.x * b.x + a.y * b.y;
}

void MyGame::update() {

	// rotate player player
	Point2 mouse = eventSystem->getMousePos();
	Vector2f diff = { player.x + ((float)player.w / 2) - camera.x - mouse.x, player.y + ((float)player.h / 2) - camera.y - mouse.y };
	targ_angle = atan2(diff.y, diff.x);

	float dtheta = targ_angle - angle;
	if (dtheta > M_PI) angle += 2 * M_PI;
	else if (dtheta < -M_PI) angle -= 2 * M_PI;
	angle += (targ_angle - angle) * 0.025;

	camera.w = gfx->getCurrentWindowSize().w;
	camera.h = gfx->getCurrentWindowSize().h;

	// try move player (fix within level)
	float bx = player.x;
	float by = player.y;
	player.x = (int)std::max((float)0, std::min((LEVEL_SIZE * TILE_SIZE) - player.w , player.x + velocity.x));
	player.y = (int)std::max((float)0, std::min((LEVEL_SIZE * TILE_SIZE) - player.h, player.y + velocity.y));

	// only move to our new posiition if its not blocked
	for (auto key : enemyShips) {
		if (key->rect.intersects(player)) {
			player.x = bx;
			player.y = by;
		}
	}

	// fix camera within level
	camera.x = std::max((float)0, std::min((float)(LEVEL_SIZE * TILE_SIZE) - camera.w, player.x - ((camera.w - player.w) / 2)));
	camera.y = std::max((float)0, std::min((float)(LEVEL_SIZE * TILE_SIZE) - camera.h, player.y - ((camera.h - player.h) / 2)));

	// tick bullets (for collision)
	for (auto key : bullets) {
		if (!key->isAlive) continue;

		for (auto ship : enemyShips) {
			if (ship->isAlive && ship->rect.intersects(key->rect)) {
				ship->isAlive = false;
				key->isAlive = false;

				sfx->playSound(ResourceManager::getSound("res/sounds/break.wav"));
				mySystem->setValue("score", mySystem->getValue<int>("score") + 200);
				remainingShips--;
			}
		}

		key->rect.x += key->velocity.x;
		key->rect.y += key->velocity.y;
	}

	if (remainingShips == 0 && !mySystem->getValue<bool>("game_win"))
		mySystem->setValue("game_win", true);

	mySystem->update(eventSystem, gfx);
}

void MyGame::render() {
	frame++;
	if (frame == 11520) frame = 0; // 64 * 180

	// draw world
	auto tilemap = ResourceManager::getTexture("res/textures/tilesheet.png");
	auto watermap = ResourceManager::getTexture("res/textures/water.png");
	for (int x = 0; x < LEVEL_SIZE; x++)
	{
		for (int y = 0; y < LEVEL_SIZE; y++)
		{
			int tile = level[x][y];
			drawTilemap(x, y, watermap, 0, frame % 64);
			if(tile != 0) drawTilemap(x, y, tilemap, tile);
		}
	}

	// draw ships
	for (auto key : enemyShips) {
		Rectangle2f shipRect = { key->rect.x, key->rect.y, key->rect.w, key->rect.h };
		shipRect.x -= camera.x;
		shipRect.y -= camera.y;
		gfx->drawTexture(key->isAlive ? ResourceManager::getTexture(mySystem->getValue<std::string>("enemy_tex"))
			: ResourceManager::getTexture("res/textures/enemy_dead.png")
			, 0, &shipRect.getSDLRect(), toDegrees(key->angle + sin((float)(key->rect.x + frame) / 20) / 10) + 90);
	}

	// draw bullets
	for (auto key : bullets) {
		if (!key->isAlive) continue;

		Rectangle2f bulletRect = { key->rect.x, key->rect.y, key->rect.w, key->rect.h };
		bulletRect.x -= camera.x;
		bulletRect.y -= camera.y;
		gfx->drawTexture(ResourceManager::getTexture("res/textures/canonball.png"), &bulletRect.getSDLRect());
	}

	// draw player
	Rectangle2f playerRect = { player.x - camera.x, player.y - camera.y, player.w, player.h };
	float playerAngle = angle + (sin((float)(player.x + frame) / 20) / 10);
	gfx->drawTexture(ResourceManager::getTexture(mySystem->getValue<std::string>("player_tex")), 0, &playerRect.getSDLRect(), toDegrees(playerAngle) + 90);
}

void MyGame::drawTilemap(int x, int y, SDL_Texture *tilemap, int tile, int scroll_offset) {
	int tx = tile % TILESHEET_X;
	int ty = tile / TILESHEET_Y;
	Rect srcRect = { (tx * TILE_SIZE_SRC) + scroll_offset, ty * TILE_SIZE_SRC, TILE_SIZE_SRC, TILE_SIZE_SRC };

	Rect tileRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
	tileRect.x -= camera.x;
	tileRect.y -= camera.y;
	gfx->drawTexture(tilemap, &srcRect.getSDLRect(), &tileRect.getSDLRect());
}

void MyGame::renderUI() {
	gfx->useFont(gameFnt);

	gfx->setDrawColor(mySystem->getValue<SDL_Color>("gui_color"));
	std::string scoreStr = mySystem->getValue<std::string>("score");
	gfx->drawText(scoreStr, 780 - scoreStr.length() * 50, camera.h - 100);

	if (mySystem->getValue<bool>("game_win"))
		gfx->drawText(mySystem->getValue<std::string>("game_win_msg"), 250, 500);

	mySystem->render(gfx);
}

EnemyShip::EnemyShip() : rect(0, 0, 0, 0) { }
Bullet::Bullet() : rect(0, 0, 0, 0) { }
