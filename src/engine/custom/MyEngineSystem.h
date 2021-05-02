#ifndef __MY_ENGINE_H__
#define __MY_ENGINE_H__

#include "../EngineCommon.h"
#include "../GraphicsEngine.h"
#include "../EventEngine.h"
#include <functional>
#include <unordered_map>
#include <queue>
#include <sstream>
#include <functional>
#include <array>

typedef std::function<void(const std::string&)> CFunc;

enum LineType {
	LINETYPE_INFO,
	LINETYPE_WARNING,
	LINETYPE_ERROR,
	LINETYPE_SUCCESS,
	LINETYPE_SYSTEM
};

struct LineEntry {
	std::string line;
	LineType type;
};

struct CFuncEntry {
	std::string help;
	CFunc function;
};

struct CVar {
	std::string value;
	CFunc callback;
};

typedef std::unordered_map<std::string, CFuncEntry> FunctionRegistry;
typedef std::unordered_map<std::string, CVar> VarRegistry;

class MyEngineSystem {
	friend class XCube2Engine;
	private:
		FunctionRegistry registryFunc;
		VarRegistry registryVar;

		std::deque<LineEntry> logBuffer;
		std::deque<std::string> cmdHistory;

		std::string inputString;

		TTF_Font* consoleFnt;

		bool isOpen = false;
		int consoleY = 0;
		int consoleScroll = 0;
		int inputCursor = 0;
		int animFrame = 0;

		void print_direct(std::string, LineType = LINETYPE_INFO);

		/* command history */
		int cursorHistory = -1;
		void readHistory();
		void saveToHistory(const std::string&);

		// evaluates a phrase (doesn't remove quotes! internal only!)
		std::string eval_loop(const std::string&);

	public:
		MyEngineSystem();

		/* engine hooks */
		void update(EventEngine*, GraphicsEngine*);
		void render(GraphicsEngine*);
		void toggle();
		void submit();

		/* console actions */
		void exec(const std::string&, bool);
		void print(const std::string&, LineType = LINETYPE_INFO);

		/* functions */
		template<typename A>
		void function(const std::string& name, A* instance, void (A::* func)(const std::string&), std::string help = "") {
			registryFunc[name].function = [=](std::string args) { (instance->*func)(args); };
			registryFunc[name].help = help;
		}
		bool callFunc(const std::string&, const std::string&);

		/* variables */
		// these must be included in this header file or we won't compile!
#pragma region Console Variables
		template <typename T>
		void variable(const std::string& name, T default)
		{
			std::ostringstream value;
			value << default;

			registryVar[name].value = value.str();
			registryVar[name].callback = NULL;
		}
		template <typename T, size_t size>
		void variable(const std::string& name, const T(&def)[size])
		{
			std::ostringstream value;
			for (size_t i = 0; i < size; i++)
			{
				if(i > 0) value << " ";
				value << def[i];
			}

			registryVar[name].value = value.str();
			registryVar[name].callback = NULL;
		}
		void variable(const std::string& name, SDL_Color color)
		{
			variable<int, 3>(name, { color.r, color.g, color.b });
		}
		template <typename A, typename T>
		void variable(const std::string &name, T default, A* instance, void (A:: * func)(const std::string&))
		{
			std::ostringstream value;
			value << default;

			registryVar[name].value = value.str();
			registryVar[name].callback = [=](std::string args) { (instance->*func)(args); };

			// call the callback right now!
			registryVar[name].callback(value.str());
		}

		template <typename T>
		void setValue(const std::string &variable, T v)
		{
			std::ostringstream value;
			value << v;
			registryVar[variable].value = value.str();

			if(registryVar[variable].callback != NULL)
				registryVar[variable].callback(value.str());
		}

		template <typename T>
		T getValue(const std::string &variable)
		{
			if (hasVariable(variable)) {
				std::istringstream ss(registryVar[variable].value);
				T value;
				ss >> value;
				return value;
			}

			return NULL;
		}
		template <typename T, size_t size>
		std::array<T, size> getValue(const std::string& variable)
		{
			std::array<T, size> output = { };
			if (hasVariable(variable)) {
				std::istringstream ss(registryVar[variable].value);
				T value;
				
				for (size_t i = 0; i < size; i ++)
				{
					ss >> output[i];
				}

				return output;
			}

			return output;
		}
		template <>
		SDL_Color getValue<SDL_Color>(const std::string& variable)
		{
			if (hasVariable(variable)) {
				auto data = getValue<int, 3>(variable);
				SDL_Color color = { (Uint8)data[0], (Uint8)data[1], (Uint8)data[2] };
				return color;
			}

			return SDL_COLOR_WHITE;
		}
		template <>
		std::string getValue<std::string>(const std::string &variable)
		{
			if (hasVariable(variable)) {
				return registryVar[variable].value;
			}

			return "";
		}
		template <>
		bool getValue<bool>(const std::string &variable)
		{
			if (hasVariable(variable)) {
				return registryVar[variable].value == "1";
			}

			return false;
		}
		bool hasVariable(const std::string &variable) {
			auto iter = registryVar.find(variable);
			return iter != registryVar.end();
		}
#pragma endregion

		// evaluates a phrase
		std::string eval(const std::string &);

		/* command controls */
		void historyUp(EventEngine*);
		void historyDown(EventEngine*);
		void autocomplete(EventEngine*);

		/* variables */
		bool getIsOpen() { return isOpen; }

		/* commands */
		void MyEngineSystem::cmd_setVariable(const std::string&);
		void MyEngineSystem::cmd_getVariable(const std::string&);
		void MyEngineSystem::cmd_echo(const std::string&);
		void MyEngineSystem::cmd_clear(const std::string&);
		void MyEngineSystem::cmd_help(const std::string&);
		void MyEngineSystem::cmd_clearHistory(const std::string&);
};

#endif