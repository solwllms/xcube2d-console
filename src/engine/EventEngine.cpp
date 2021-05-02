#include "EventEngine.h"

EventEngine::EventEngine() : running(true) {
	for (int i = 0; i < Key::LAST; ++i) {
		keys[i] = false;
	}

	buttons[Mouse::BTN_LEFT] = false;
	buttons[Mouse::BTN_RIGHT] = false;
}

EventEngine::~EventEngine() {}

void EventEngine::pollEvents() {
	std::copy(std::begin(keys), std::end(keys), std::begin(keysLast));

	// clamp the cursor position - just in case!
	if (cursor > inputString.length()) cursor = inputString.length();
	else if (cursor < 0) cursor = 0;

	while (SDL_PollEvent(&event)) {
		auto key = event.key.keysym.sym;
		if (event.type == SDL_TEXTINPUT)
		{
			// not the back quote OR one of our CTRL shortcuts
			if (event.text.text[0] != '`' &&
				!(SDL_GetModState() & KMOD_CTRL && (event.text.text[0] == 'V' || event.text.text[0] == 'v' ||
					event.text.text[0] == 'C' || event.text.text[0] == 'c')))
			{
				std::string insert = event.text.text;
				inputString.insert(cursor, insert);
				cursor += insert.length();
			}
		}
		else {
			if (event.type == SDL_KEYDOWN) {
				if (key == SDLK_BACKSPACE && inputString.size() > 0 && cursor > 0) {
					inputString = inputString.substr(0, cursor - 1) + inputString.substr(cursor, inputString.length());
					cursor--;
				}
				if (key == SDLK_DELETE && inputString.size() > 0 && cursor < inputString.size()) {
					inputString = inputString.substr(0, cursor) + inputString.substr(cursor + 1, inputString.length());
				}
				else if (SDL_GetModState() & KMOD_CTRL)
				{
					// copy + paste
					if (key == SDLK_v) {
						std::string insert = SDL_GetClipboardText();
						inputString.insert(cursor, insert);
						cursor += insert.length();
					}
					else if(key == SDLK_c) SDL_SetClipboardText(inputString.c_str());
				}
			}
			if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
				updateKeys(key, event.type == SDL_KEYDOWN);
			}

			if (event.type == SDL_QUIT) {
				keys[QUIT] = true;
			}
		}

		buttons[Mouse::BTN_LEFT]  = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
		buttons[Mouse::BTN_RIGHT] = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
	}
}

void EventEngine::updateKeys(const SDL_Keycode &key, bool keyDown) {
	Key index;

	switch (key) {
		case SDLK_RIGHT:	index = Key::RIGHT; break;
		case SDLK_d:		index = Key::D; break;
		case SDLK_LEFT:		index = Key::LEFT; break; 
		case SDLK_a:		index = Key::A; break;
		case SDLK_UP:		index = Key::UP; break;
		case SDLK_w:		index = Key::W; break;
		case SDLK_DOWN:		index = Key::DOWN; break;
		case SDLK_s:		index = Key::S; break;
		case SDLK_ESCAPE:	index = Key::ESC; break;
		case SDLK_SPACE:	index = Key::SPACE; break;
		case SDLK_BACKQUOTE:index = Key::CONSOLE; break;
		case SDLK_RETURN:	index = Key::RETURN; break;
		case SDLK_TAB:		index = Key::TAB; break;
		default:
			return;	// we don't care about other keys, at least now
	}

	keys[index] = keyDown;
}

void EventEngine::setPressed(Key key) {
    keys[key] = true;
}

void EventEngine::setPressed(Mouse btn) {
    buttons[btn] = true;
}

bool EventEngine::isPressed(Key key) {
	return keys[key];
}

bool EventEngine::isReleased(Key key) {
	return !keys[key] && keysLast[key];
}

bool EventEngine::isPressed(Mouse btn) {
	return buttons[btn];
}

void EventEngine::setMouseRelative(bool b) {
	if (SDL_SetRelativeMouseMode(b ? SDL_TRUE : SDL_FALSE) < 0) {
#ifdef __DEBUG
		debug("Warning: SDL_SetRelativeMouseMode() isn't supported");
#endif
	}
}

Point2 EventEngine::getMouseDPos() {
	Point2 mouseDPos;
	SDL_GetRelativeMouseState(&mouseDPos.x, &mouseDPos.y);
	return mouseDPos;
}

Point2 EventEngine::getMousePos() {
	Point2 pos;
	SDL_GetMouseState(&pos.x, &pos.y);
	return pos;
}
