#include "MyGame.h"

MyGame::MyGame() : AbstractGame(), player(0, 0, 33, 56), camera(0, 0, 0, 0) {
	gameFnt = ResourceManager::loadFont("res/fonts/arial.ttf", 72);
	gfx->useFont(gameFnt);
	gfx->setVerticalSync(true);

	ResourceManager::loadTexture("res/textures/tilesheet.png", magicPink);
	ResourceManager::loadTexture("res/textures/water.png", SDL_COLOR_BLACK);
	ResourceManager::loadTexture("res/textures/player.png", magicPink);
	ResourceManager::loadTexture("res/textures/canonball.png", magicPink);
	ResourceManager::loadTexture("res/textures/player.png", magicPink);
	ResourceManager::loadTexture("res/textures/enemy.png", magicPink);
	ResourceManager::loadTexture("res/textures/enemy_dead.png", magicPink);

	for (int x = 0; x < LEVEL_SIZE; x++)
	{
		for (int y = 0; y < LEVEL_SIZE; y++)
		{
			level[x][y] = getRandom(0, 8) == 2 ? getRandom(1, 3) : 0;
		}
	}

	mySystem->variable("window_title", "Sol Williams - Demo game", this, &MyGame::setTitle);

	mySystem->variable("score", 0);
	mySystem->variable("lives", 3);
	mySystem->variable("num_keys", 5);

	mySystem->variable("game_won", false);
	mySystem->variable("game_win_msg", "YOU WON!");

	mySystem->variable("player_speed", 1);
	mySystem->variable("gui_color", SDL_COLOR_RED);

    for (int i = 0; i < mySystem->getValue<int>("num_keys"); i++) {
		spawnShip("");
    }

	player.x = (gfx->getCurrentWindowSize().w - 66) / 2;
	player.y = (gfx->getCurrentWindowSize().h - 113) / 2;

	mySystem->print("World generated!", LINETYPE_SUCCESS);
}

MyGame::~MyGame() {

}

void MyGame::setTitle(const std::string& s) {
	gfx.get()->setWindowTitle(s);
}

void MyGame::spawnShip(const std::string& s) {
	std::shared_ptr<EnemyShip> k = std::make_shared<EnemyShip>();
	k->isAlive = true;
	k->angle = getRandom(0, M_PI * 2);
	k->rect = Rect(getRandom(0, (LEVEL_SIZE - 3) * TILE_SIZE), getRandom(0, (LEVEL_SIZE - 3) * TILE_SIZE), 33, 56);
	enemyShips.push_back(k);

	mySystem->print("spawned ship at (x: "+std::to_string(k->rect.x)+", y: "+ std::to_string(k->rect.y) + ")");
}

void MyGame::fire(const std::string& s) {
	std::shared_ptr<Bullet> k = std::make_shared<Bullet>();
	k->isAlive = true;
	k->rect = Rect(player.x + ((float)player.w / 2), player.y + ((float)player.h / 2), 16, 16);
	k->velocity = Vector2i(cos(angle) * -10, sin(angle) * -10);
	bullets.push_back(k);

	mySystem->print("spawned bullet at (x: " + std::to_string(k->rect.x) + ", y: " + std::to_string(k->rect.y) + ")");
}

void MyGame::handleKeyEvents() {
	int speed = mySystem->getValue<int>("player_speed");

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

	if (eventSystem->isReleased(Key::SPACE)) {
		fire("");
	}
}

float dot(Vector2f a, Vector2f b)  //calculates dot product of a and b
{
	return a.x * b.x + a.y * b.y;
}

float mag(Vector2f a)  //calculates magnitude of a
{
	return std::sqrt(a.x * a.x + a.y * a.y);
}

void MyGame::update() {

	Point2 mouse = eventSystem->getMousePos();
	Vector2f diff = { player.x + ((float)player.w / 2) - camera.x - mouse.x, player.y + ((float)player.h / 2) - camera.y - mouse.y };
	angle = atan2(diff.y, diff.x);

	camera.w = gfx->getCurrentWindowSize().w;
	camera.h = gfx->getCurrentWindowSize().h;

	float bx = player.x;
	float by = player.y;
	player.x = std::max(0, std::min(LEVEL_SIZE * TILE_SIZE, player.x + velocity.x));
	player.y = std::max(0, std::min(LEVEL_SIZE * TILE_SIZE, player.y + velocity.y));

	for (auto key : enemyShips) {
		if (key->rect.intersects(player)) {
			player.x = bx;
			player.y = by;
		}
	}

	camera.x = std::max(0, std::min((LEVEL_SIZE * TILE_SIZE) - camera.w, player.x - ((camera.w - player.w) / 2)));
	camera.y = std::max(0, std::min((LEVEL_SIZE * TILE_SIZE) - camera.h, player.y - ((camera.h - player.h) / 2)));

	for (auto key : bullets) {
		if (!key->isAlive) continue;

		for (auto ship : enemyShips) {
			if (ship->rect.intersects(key->rect)) {
				ship->isAlive = false;
				key->isAlive = false;
			}
		}

		key->rect.x += key->velocity.x;
		key->rect.y += key->velocity.y;
	}

	velocity.x = 0;
    velocity.y = 0;

	mySystem->update(eventSystem.get(), gfx.get());
}

void MyGame::render() {
	frame_timer++;
	frame = frame_timer / 15;
	if (frame_timer == 60) frame_timer = 0;

	auto tilemap = ResourceManager::getTexture("res/textures/tilesheet.png");
	auto watermap = ResourceManager::getTexture("res/textures/water.png");
	for (int x = 0; x < LEVEL_SIZE; x++)
	{
		for (int y = 0; y < LEVEL_SIZE; y++)
		{
			int tile = level[x][y];
			drawTilemap(x, y, watermap, frame % 2);
			if(tile != 0) drawTilemap(x, y, tilemap, tile);
		}
	}

	gfx->setDrawColor(mySystem->getValue<SDL_Color>("key_color"));
	for (auto key : enemyShips) {

		Rect shipRect = { key->rect.x, key->rect.y, key->rect.w, key->rect.h };
		shipRect.x -= camera.x;
		shipRect.y -= camera.y;
		gfx->drawTexture(key->isAlive ? ResourceManager::getTexture("res/textures/enemy.png")
			: ResourceManager::getTexture("res/textures/enemy_dead.png")
			, 0, &shipRect.getSDLRect(), toDegrees(key->angle) + 90);
	}

	for (auto key : bullets) {
		if (!key->isAlive) continue;

		Rect bulletRect = { key->rect.x, key->rect.y, key->rect.w, key->rect.h };
		bulletRect.x -= camera.x;
		bulletRect.y -= camera.y;
		gfx->drawTexture(ResourceManager::getTexture("res/textures/canonball.png"), &bulletRect.getSDLRect());
	}

	Rect playerRect = { player.x - camera.x, player.y - camera.y, player.w, player.h };
	gfx->drawTexture(ResourceManager::getTexture("res/textures/player.png"), 0, &playerRect.getSDLRect(), toDegrees(angle) + 90);
}

void MyGame::drawTilemap(int x, int y, SDL_Texture *tilemap, int tile) {
	int tx = tile % TILESHEET_X;
	int ty = tile / TILESHEET_Y;
	Rect srcRect = { tx * TILE_SIZE_SRC, ty * TILE_SIZE_SRC, TILE_SIZE_SRC, TILE_SIZE_SRC };

	Rect tileRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
	tileRect.x -= camera.x;
	tileRect.y -= camera.y;
	gfx->drawTexture(tilemap, &srcRect.getSDLRect(), &tileRect.getSDLRect());
}

void MyGame::renderUI() {
	gfx->useFont(gameFnt);

	gfx->setDrawColor(mySystem->getValue<SDL_Color>("gui_color"));
	std::string scoreStr = mySystem->getValue<std::string>("score");
	gfx->drawText(scoreStr, 780 - scoreStr.length() * 50, 25);

	if (mySystem->getValue<bool>("game_won"))
		gfx->drawText(mySystem->getValue<std::string>("game_win_msg"), 250, 500);

	mySystem->render(gfx.get());
}

EnemyShip::EnemyShip() : rect(0, 0, 0, 0) { }
Bullet::Bullet() : rect(0, 0, 0, 0) { }
