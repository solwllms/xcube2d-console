#ifndef __EVENT_ENGINE_H__
#define __EVENT_ENGINE_H__

#include <string>
#include <thread>

#include <SDL.h>

#include "EngineCommon.h"
#include "GameMath.h"

enum Key {
	W, S, A, D, ESC, SPACE, UP, DOWN, LEFT, RIGHT, QUIT, CONSOLE, RETURN, TAB, LAST
};

enum Mouse {
	BTN_LEFT, BTN_RIGHT, BTN_LAST
};

class EventEngine {
	friend class XCube2Engine;
	private:
		bool running;
		SDL_Event event;
		bool keys[Key::LAST];
		bool keysLast[Key::LAST];
		bool buttons[Mouse::BTN_LAST];

		std::string inputString;
		int cursor = 0;

		void updateKeys(const SDL_Keycode &, bool);

		EventEngine();
	public:
		~EventEngine();

		/**
		* Equivalent to calling SDL_PollEvent()
		*/
		void pollEvents();
		
		bool isPressed(Key);
		bool isReleased(Key);
		bool isPressed(Mouse);

		std::string getInputString() { return inputString; }
		void setInputString(std::string value) { inputString = value; }
		void appendInputString(std::string value) { inputString += value; }
		void clearInputString() { inputString = ""; }

		void cursorLeft() {
			if (cursor > 0) {
				cursor--;
			}
		}
		void cursorRight() {
			if (inputString.length() > 0 && cursor <= inputString.length() - 1) {
				cursor++;
			}
		}
		void setCursor(int value) { cursor = value; }
		int getCursor() { return cursor; }
    
        /**
         * Software emulation of keypresses
         */
        void setPressed(Key);
        void setPressed(Mouse);
	
		void setMouseRelative(bool);

		/**
		* Returns mouse's delta position
		* It's the difference between current and
		* previous mouse positions
		*
		*/
		Point2 getMouseDPos();

		/**
		* Returns current mouse position relative to the window
		*/
		Point2 getMousePos();
};

#endif
