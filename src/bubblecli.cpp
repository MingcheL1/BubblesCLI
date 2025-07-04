#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sys/ioctl.h>
#include <unistd.h>
#include <map>
#include <filesystem>

using namespace std;

struct Bubble {
    int x, y;
    int drift;
    string symbol;
};

bool useColor = true;
bool useBigBubbles = false;
bool showHelp = false;


vector<string> defaultColoredSymbols = {
    "\033[38;5;231m●", "\033[38;5;195m○",
    "\033[38;5;159m◌", "\033[38;5;189m◍",
    "\033[38;5;252m◎", "\033[38;5;250m◉"
};

vector<string> bigBubbles = {
    "( )", "(@)", "Ooo", "0","oOo", "OoO", "(○)"
};

void printHelp() {
    cout << R"(
🫧 BubblesCLI 🫧

Usage:   bubblescli [options]

Options:
  -h, --help        help
  --no-color        Disable color 
  -b, --big         Use big bubbles 
)";
}

void parseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") showHelp = true;
        else if (arg == "--no-color") useColor = false;
        else if (arg == "-b" || arg == "--big") useBigBubbles = true;
    }
}

// Default config
int FPS = 10;
int SPAWN_RATE = 3;
int MAX_BUBBLES = 100;
vector<string> bubbleSymbols = {
    "\033[38;5;231m●", "\033[38;5;195m○",
    "\033[38;5;159m◌", "\033[38;5;189m◍",
    "\033[38;5;252m◎", "\033[38;5;250m◉"
};

string CONFIG_PATH = getenv("HOME") + string("/.config/bubblescli/config.ini");

int getTerminalWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

int getTerminalHeight() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

void clearScreen() {
    cout << "\033[2J\033[H";
}

void moveCursorHome() {
    cout << "\033[H";
}

void render(const vector<Bubble>& bubbles, int width, int height) {

    vector<vector<string>> screen(height, vector<string>(width, " "));

    for (const auto& b : bubbles) {
        if (b.y >= 0 && b.y < height && b.x >= 0 && b.x < width) {
            screen[b.y][b.x] = b.symbol;
        }
    }

    moveCursorHome();
    for (const auto& row : screen) {
        for (const auto& cell : row) cout << cell;
        cout << "\033[0m\n";
    }
}


void parseConfig() {
    ifstream config(CONFIG_PATH);
    if (!config) return;

    string line;
    while (getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream iss(line);
        string key, value;
        if (getline(iss, key, '=') && getline(iss, value)) {
            if (key == "fps") FPS = stoi(value);
            else if (key == "spawn_rate") SPAWN_RATE = stoi(value);
            else if (key == "max_bubbles") MAX_BUBBLES = stoi(value);
            else if (key == "symbols") {
                bubbleSymbols.clear();
                istringstream vs(value);
                string symbol;
                while (getline(vs, symbol, ',')) {
                    bubbleSymbols.push_back("\033[38;5;231m" + symbol);
                }
            }
        }
    }
}

void generateDefaultConfig() {
    if (!filesystem::exists(CONFIG_PATH)) {
        filesystem::create_directories(filesystem::path(CONFIG_PATH).parent_path());
        ofstream config(CONFIG_PATH);
        config << "# BubblesCLI config\n";
        config << "fps=10\n";
        config << "spawn_rate=3\n";
        config << "max_bubbles=100\n";
        config << "symbols=●,○,◌,◍,◎,◉\n";
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));
    parseArgs(argc, argv);

    if (showHelp) {
        printHelp();
        return 0;
    }

    generateDefaultConfig();
    parseConfig();

    if (!useColor) {
        for (auto& s : bubbleSymbols) {
            s = "o";
        }
    }

    if (useBigBubbles) {
        bubbleSymbols = bigBubbles;
    } else if (useColor) {
        bubbleSymbols = defaultColoredSymbols;
    }

    vector<Bubble> bubbles;
    int tick = 0;

    while (true) {
        int width = getTerminalWidth();
        int height = getTerminalHeight();

        if (tick % SPAWN_RATE == 0 && (int)bubbles.size() < MAX_BUBBLES) {
            Bubble b;
            b.x = rand() % width;
            b.y = height - 1;
            b.drift = (rand() % 3) - 1;
            b.symbol = bubbleSymbols[rand() % bubbleSymbols.size()];
            bubbles.push_back(b);
        }

        for (auto& b : bubbles) {
            b.y -= 1;
            b.x += b.drift + ((rand() % 3) - 1);
            b.x = max(0, min(b.x, width - 1));
        }

        bubbles.erase(
            remove_if(bubbles.begin(), bubbles.end(), [](const Bubble& b) { return b.y < 0; }),
            bubbles.end()
        );

        render(bubbles, width, height);
        this_thread::sleep_for(chrono::milliseconds(1000 / FPS));
        tick++;
    }

    return 0;
}


