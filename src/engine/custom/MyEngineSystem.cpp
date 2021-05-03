#include "MyEngineSystem.h"
#include "../AbstractGame.h"
#include <fstream>
#include <iomanip>


using namespace std;

const int CONSOLE_MAX_BUFFER = 128;

MyEngineSystem::MyEngineSystem() {
    consoleFnt = ResourceManager::loadFont("res/fonts/ubuntumono.ttf", 16);

    print_direct("Sol's Console v1 for XCube2d", LINETYPE_SYSTEM);
    print_direct("by Sol Williams for CI517");
    print_direct("");

    readHistory();

    function("set", this, &MyEngineSystem::cmd_setVariable, "set a variable to a value");
    function("value", this, &MyEngineSystem::cmd_getVariable, "echo the value of a variable");
    function("if", this, &MyEngineSystem::cmd_if, "perfom a conditional command");
    function("exec", this, &MyEngineSystem::cmd_exec, "executes a script file");
    function("echo", this, &MyEngineSystem::cmd_echo, "print a message to the console");
    function("clear", this, &MyEngineSystem::cmd_clear, "clear the console");
    function("help", this, &MyEngineSystem::cmd_help, "print this message");
    function("clearhistory", this, &MyEngineSystem::cmd_clearHistory, "clear the command history log");
    function("quit", this, &MyEngineSystem::cmd_quit, "exit the game");
    function("play", this, &MyEngineSystem::cmd_playSound, "play a sound");

    variable("con_height", 50);
    variable("echo_mode", LINETYPE_INFO);

    SDL_StopTextInput();
    print("Console initialised.", LINETYPE_SUCCESS);
}

void MyEngineSystem::cmd_playSound(const std::string& s) {
    if (empty(s)) return;
    XCube2Engine::getInstance()->getAudioEngine()->playSound(ResourceManager::getSound(s));
}

void MyEngineSystem::cmd_quit(const std::string&) {
    XCube2Engine::quit();
}

void MyEngineSystem::print_direct(string line, LineType type) {
    LineEntry entry;
    entry.line = line;
    entry.type = type;

    logBuffer.push_front(entry);
}

void MyEngineSystem::print(const std::string& message, LineType type) {
    std::string newline = "\n";

    char tstamp[21];
    time_t now = time(NULL);
    struct tm curtime = *localtime(&now);
    strftime(tstamp, sizeof(tstamp), "[%x %X] ", &curtime);
    auto line = tstamp + message + "\n";

    size_t start = 0;
    size_t end = line.find(newline);
    while (end != std::string::npos)
    {
        string print = line.substr(start, end - start);
        std::cout << print + "\n";

        LineEntry entry;
        entry.line = print;
        entry.type = type;

        if (logBuffer.size() == CONSOLE_MAX_BUFFER - 1) logBuffer.pop_back();
        logBuffer.push_front(entry);

        start = end + newline.length();
        end = line.find(newline, start);
    }
}

void MyEngineSystem::render(shared_ptr<GraphicsEngine> gfx) {
    animFrame++;

    Dimension2i curWindowSize = gfx->getCurrentWindowSize();
    gfx->setDrawColor(SDL_COLOR_BLACK);
    gfx->fillRect(0, 0, curWindowSize.w, consoleY);

    gfx->setDrawColor(SDL_COLOR_BLUE);
    gfx->fillRect(0, consoleY - 1, curWindowSize.w, 1);

    gfx->setDrawColor(SDL_COLOR_BLACK);
    gfx->fillRect(5, consoleY - 30, curWindowSize.w - 10, 24);

    gfx->useFont(consoleFnt);

    // wrap lines around screen
    std::deque<LineEntry> lines;
    for (size_t i = logBuffer.size(); i > consoleScroll; i--)
    {
        LineEntry entry = logBuffer.at(i - 1);
        string line = logBuffer.at(i - 1).line;

        int w;
        bool fits = false;
        bool done = false;
        while (!done) {
            int phrase_start = 0;
            int phrase_end = line.length();

            // keep whittling down until it fits to size
            while (!fits) {
                if (!TTF_SizeText(consoleFnt, line.substr(phrase_start, phrase_end).c_str(), &w, NULL)) {
                    fits = w < curWindowSize.w - 5;
                    if (!fits) {
                        phrase_end--;
                    }
                }
                else fits = true; // failed to measure for some reason
            }

            // copy this info across
            LineEntry push_entry;
            push_entry.line = line.substr(phrase_start, phrase_end);
            push_entry.type = entry.type;

            lines.push_front(push_entry);

            line = line.substr(phrase_end, line.length() - 1);
            done = line.length() == 0;
        }
    }

    // print lines to screen
    int y = -34;
    string line;
    for (size_t i = 0; i < lines.size(); i++)
    {
        y -= 16;
        LineEntry entry = lines.at(i);
        line = entry.line;
        if (line.empty()) continue;

        if (entry.type == LINETYPE_INFO)
            gfx->setDrawColor(SDL_COLOR_WHITE);
        else if (entry.type == LINETYPE_ERROR)
            gfx->setDrawColor(SDL_COLOR_RED);
        else if (entry.type == LINETYPE_WARNING)
            gfx->setDrawColor(SDL_COLOR_YELLOW);
        else if (entry.type == LINETYPE_SUCCESS)
            gfx->setDrawColor(SDL_COLOR_GREEN);
        else if (entry.type == LINETYPE_SYSTEM)
            gfx->setDrawColor(SDL_COLOR_AQUA);

        gfx->drawText(line, 5, consoleY + y);
    }
        
    gfx->setDrawColor(SDL_COLOR_WHITE);
    gfx->drawText("> " + inputString, 10, consoleY - 28);

    if ((animFrame % 60) < 30) {
        gfx->drawText("_", 10 + TTF_FontFaceIsFixedWidth(consoleFnt) * 2 * (inputCursor + 2), consoleY - 28);
    }
}

void MyEngineSystem::update(std::shared_ptr<EventEngine> event, std::shared_ptr<GraphicsEngine> gfx) {
    int max = (gfx->getCurrentWindowSize().h * (getValue<float>("con_height") / 100));
    float targ = isOpen ? max : 0;

    if(consoleY < targ) consoleY += 20;
    else if (consoleY > targ) consoleY -= 20;

    inputCursor = event->getCursor();
    inputString = event->getInputString();
}

void MyEngineSystem::submit() {
    exec(inputString, true);
    cursorHistory = -1;
    inputString = "";
}

void MyEngineSystem::toggle() {
    isOpen = !isOpen;

    if(isOpen) SDL_StartTextInput();
    else SDL_StopTextInput();
}

void MyEngineSystem::exec(const std::string& input, bool userInput) {
    if (userInput) {
        print("> " + input);
        saveToHistory(input);
    }

    std::string newline = ";";
    std::string line = input + newline;

    userExec = userInput;

    size_t start = 0;
    size_t end = line.find(newline);
    while (end != std::string::npos)
    {
        auto command = line.substr(start, end - start);

        string cmd, args = "";

        int space = command.find(' ');
        if (space != std::string::npos) {
            cmd = command.substr(0, space);
            args = command.substr(space + 1, command.size() - 1);
        }
        else cmd = command;

        if (registryFunc.find(cmd) != registryFunc.end()) {
            callFunc(cmd, args);
        }
        else {
            string value = eval(input);
            if (value.empty()) {
                print("ERROR: '" + cmd + "' is not a valid command.", LINETYPE_ERROR);
                return;
            }
            else print(value);
        }

        start = end + newline.length();
        end = line.find(newline, start);
    }
}

bool MyEngineSystem::callFunc(const std::string& functionName, const std::string& args)
{
    auto iter = registryFunc.find(functionName);
    if (iter != registryFunc.end()) {
        iter->second.function(args);
        return true;
    }

    return false;
}

// based-off of a tokeniser by xinaiz
// https://stackoverflow.com/questions/34653318/split-a-string-by-but-ignore-text-inside-quotes-in-the-string-c-using-boos
inline vector<string> tokenise(const string& str) {

    vector<string> v;
    bool flag_quote = false;
    v.push_back("");
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\"')
            flag_quote = flag_quote ? false : true;

        if ((str[i] == '(' || str[i] == ')') && !flag_quote) {
            v.push_back(string(1, str[i]));
            v.push_back("");
        }
        else if (str[i] == ' ' && !flag_quote)
            v.push_back("");
        else
            v[v.size() - 1] += str[i];
    }

    return v;
}

inline string as_string(double d) {
    stringstream ss;
    ss << d;
    return ss.str();
}

string MyEngineSystem::eval(const std::string& expr) {
    string token = eval_loop(expr);
    token.erase(remove(token.begin(), token.end(), '\"'), token.end());
    return token;
}


// heavily modified code - but based off of concepts of
// https://stackoverflow.com/questions/9329406/evaluating-arithmetic-expressions-from-string-in-c
string MyEngineSystem::eval_loop(const std::string& expr) {
    double e1, e2;    
    vector<string> series = tokenise(expr);
    string token = "";

    if (series.size() == 0) return "";
    else if (series.size() == 1) {
        token = series[0];

        // variables
        if (token[0] == '$') {
            string var = token.substr(1, token.size() - 1);
            if (hasVariable(var)) {
                token = getValue<string>(var);
                try {
                    double a = stod(token);
                    return as_string(a);
                }
                catch (...) {
                    return "\"" + token + "\"";
                }
            }
            else {
                print("variable '" + var + "' not found.");
                return "";
            }
        }
    }
    else {
        for (int i = 0; i < series.size(); i++) {

            if (series[i].empty() || series[i] == "(" || series[i] == ")") continue;

            if (i < 0) token += " ";
            token += eval_loop(series[i]);
        }
    }

    if (token.length() == 1) return token;

    bool flag_quote = false;
    for (int i = 0; i < token.length(); i++)
    {
        string token1 = token.substr(0, i);
        string token2 = token.substr(i + 1, token.length() - i - 1);
        if (token[i] == '\"') flag_quote = flag_quote ? false : true;
        if (token[i] == ' ' || flag_quote) { continue;  }
        else if (token[i] == '+')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return as_string(e1 + e2);
            }
            catch (const std::invalid_argument&) {
                return eval_loop(token1) + eval_loop(token2);
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
        else if (token[i] == '-')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return as_string(e1 - e2);
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return "";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
    }

    for (int i = 0; i < token.length(); i++)
    {
        string token1 = token.substr(0, i);
        string token2 = token.substr(i + 1, token.length() - i - 1);
        if (token[i] == '\"') flag_quote = flag_quote ? false : true;
        if (token[i] == ' ' || flag_quote) { continue; }
        else if (token[i] == '/')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return as_string(e1 / e2);
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return "";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        } 
        else if (token[i] == '*')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return as_string(e1 * e2);
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return "";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
    }

    for (int i = 0; i < token.length(); i++)
    {
        string token1 = token.substr(0, i);
        string token2 = token.substr(i + 1, token.length() - i - 1);
        if (token[i] == '\"') flag_quote = flag_quote ? false : true;
        if (token[i] == ' ' || flag_quote) { continue; }
        else if (token[i] == '=')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return (e1 == e2) ? "1" : "0";
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return (token1 == token2) ? "1" : "0";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
        else if (token[i] == '>')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return (e1 > e2) ? "1" : "0";
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return "";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
        else if (token[i] == '<')
        {
            try {
                e1 = stod(eval_loop(token1));
                e2 = stod(eval_loop(token2));
                return (e1 < e2) ? "1" : "0";
            }
            catch (const std::invalid_argument&) {
                //print("error: could not evaluate");
                return "";
            }
            catch (const std::out_of_range&) {
                //print("error: out of range");
                return "";
            }
        }
    }

    try {
        return as_string(stod(token));
    }
    catch (const std::invalid_argument&) {
        //print("error: could not evaluate");
        if (token[0] == '"') return token;
        return "";
    }

    //return token.c_str();
}

void MyEngineSystem::historyUp(std::shared_ptr<EventEngine> event) {
    int size = cmdHistory.size();
    if (size > 0 && cursorHistory < size - 1) {
        cursorHistory++;
        event->setInputString(cmdHistory.at(cursorHistory));
        event->setCursor(event->getInputString().length());
    }
}
void MyEngineSystem::historyDown(std::shared_ptr<EventEngine> event) {
    if (cursorHistory > -1) {
        cursorHistory--;
        if (cursorHistory == -1) event->clearInputString();
        else event->setInputString(cmdHistory.at(cursorHistory));
        event->setCursor(event->getInputString().length());
    }
}
void MyEngineSystem::autocomplete(std::shared_ptr<EventEngine> event) {
    string search = event->getInputString();
    if (inputCursor != search.length()) return;
    string match;

    // autocomplete variable names
    auto v = tokenise(search);
    size_t varstart = v[v.size() - 1].find('$');
    if (varstart != string::npos) {
        search = v[v.size() - 1];
        for each (auto var in registryVar)
        {
            string name = "$" + var.first;
            if (name.compare(0, search.size(), search) == 0 && (match.empty() || name.length() < match.length())) {
                match = var.first;
            }
        }

        // if we've got a match, add it to our input and move the cursor
        if (!match.empty()) {
            string input = event->getInputString();
            input = input.substr(0, input.find(v[v.size() - 1]) + varstart) + "$" + match;
            event->setCursor(event->getCursor() + match.length());
            event->setInputString(input);
        }
    }
    // autocomplete function names
    else if (match.empty() && event->getInputString().find(' ') == string::npos) {
        // there's no support for the lower_bound funcs with an unordered_map!
        for each (auto var in registryFunc)
        {
            string name = var.first;
            if (name.compare(0, search.size(), search) == 0 && (match.empty() || name.length() < match.length())) {
                match = var.first;
            }
        }

        // if we've got a match, add it to our input and move the cursor
        if (!match.empty()) {
            event->setInputString(match);
            event->setCursor(event->getCursor() + match.length() - 1);
        }
    }
}

void MyEngineSystem::readHistory() {
    std::string line;
    std::ifstream file;
    file.open("history.txt");
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (!line.empty()) {
                cmdHistory.push_front(line);
            }
        }
        file.close();
    }
}

void MyEngineSystem::saveToHistory(const std::string& line) {
    if (line.empty()) return;
    cmdHistory.push_front(line);

    ofstream file;
    file.open("history.txt", std::ofstream::out | std::ofstream::app);
    file << "\n" + line;
    file.close();
}

void MyEngineSystem::cmd_setVariable(const std::string& command) {
    string cmd = command;

    string var;
    string value;
    int space = cmd.find(' ');
    if (space != string::npos) {
        var = cmd.substr(0, space);
        value = cmd.substr(space + 1, cmd.size() - 1);
    }
    else {
        print("set [VARIABLE] [VALUE]");
        return;
    }

    value = eval(value);
    if (value.empty()) {
        print("error: could not evaluate", LINETYPE_ERROR);
        return;
    }

    var.erase(remove(var.begin(), var.end(), '$'), var.end());
    setValue(var, value);

    if(userExec)
        print("set $" + var + " to \"" + value + "\"");
}

void MyEngineSystem::cmd_getVariable(const std::string& command) {
    string cmd = command;
    cmd.erase(remove(cmd.begin(), cmd.end(), '$'), cmd.end());

    if (!cmd.empty()) {
        print(getValue<string>(cmd));
    }
    else {
        print("get [VARIABLE]");
        return;
    }
}

// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
inline std::string trim(const std::string& s)
{
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c); }).base();
    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

// from https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
inline vector<string> split(const std::string& phrase, char split) {
    stringstream ss(phrase);
    vector<string> result;

    while (ss.good())
    {
        string substr;
        getline(ss, substr, split);
        substr = trim(substr);
        result.push_back(substr);
    }
    
    return result;
}

void MyEngineSystem::cmd_if(const std::string& command) {
    if (command.empty()) {
        print("if [CONDITION] [TRUE CMD] [ELSE CMD]");
        return;
    }

    string condition, cmd_true, cmd_false;
    vector<string> series = split(command, ',');
    for (int i = 0; i < series.size(); i++)
    {
        if (series[i].empty()) continue;

        if (condition.empty()) condition = series[i];
        else if (cmd_true.empty()) cmd_true = series[i];
        else if (cmd_false.empty()) cmd_false = series[i];
    }

    if (condition.empty() || cmd_true.empty()) {
        print("if [CONDITION] [TRUE CMD] (ELSE CMD)");
        return;
    }

    string result = eval(condition);
    if (result != "0") exec(cmd_true, false);
    else if(!cmd_false.empty()) exec(cmd_false, false);
}

void MyEngineSystem::cmd_echo(const std::string& command) {
    if(command.empty()) {
        print("echo [MESSAGE]");
        return;
    }

    string value = eval(command);
    if (value.empty()) {
        print("error: could not evaluate", LINETYPE_ERROR);
        return;
    }

    LineType mode = LINETYPE_INFO;
    int v = getValue<int>("echo_mode");
    if (v >= 0 && v < LINETYPE_MAX)
        mode = (LineType) v;
    print(value, mode);
}

void MyEngineSystem::cmd_clear(const std::string& command) {
    logBuffer.clear();
}

void MyEngineSystem::cmd_help(const std::string& command) {

    for each (auto var in registryFunc)
    {
        string name = var.first;

        // does match search?
        if (!command.empty() && name.compare(0, command.size(), command) != 0) continue;

        // pad right
        name.insert(name.end(), 32 - name.size(), ' ');

        print(name + var.second.help);
    }
}

void MyEngineSystem::cmd_clearHistory(const std::string& command) {
    cmdHistory.clear();

    ofstream file;
    file.open("history.txt", std::ofstream::out);
    file << "";
    file.close();

    print("history cleared.");
}

void MyEngineSystem::cmd_exec(const std::string& command) {
    if (command.empty()) {
        print("exec [FILE]");
        return;
    }

    std::string line;
    std::ifstream file;
    file.open(command);
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (!line.empty()) {
                exec(line, false);
            }
        }
        file.close();
    }
}