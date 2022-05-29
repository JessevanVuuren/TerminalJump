#include <Windows.h>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std::chrono;
using namespace std;

const int screenHeight = 30;
const int screenWidth = 100;

const int screenSize = screenHeight * screenWidth;

string *render = new string[screenSize];
char gameChar[screenHeight * screenWidth];

string printT(string, int);
string getFloorChar(int);

bool groundCheck(int);

void animateScreen(string, int);
void loadMapFromFile();
void playerMovement();
void playerEnvCheck();
void renderGame();
void sleep(int);

int character[2];

bool playerJump = false;
int startJumpPos;
int jumpInt = -10;
bool bounce = true;
int doubleJump = 0;

int aminSpeed = 0;
int loadLevel = 0;

bool PlayerLock = true;
bool gameWon = false;

int main() {
    loadMapFromFile();

    while (1) {
        aminSpeed++;
        renderGame();
        playerMovement();
        playerEnvCheck();
        sleep(10);
    }
    return 0;
}

void playerMovement() {
    if (playerJump) {
        int JumpTo = startJumpPos - floor((-jumpInt * jumpInt + 100) * 0.06);
        if (!groundCheck(character[0] - 1)) {
            character[0] = JumpTo;
        } else {
            playerJump = false;
        }
        jumpInt++;

        if (groundCheck(JumpTo + 1) && jumpInt > 0) {
            playerJump = false;
        }

        if (jumpInt > 10) {
            playerJump = false;
            jumpInt = -10;
        }
    } else {
        if (!groundCheck(character[0] + 1)) {
            character[0] += 1;
        } else {
            doubleJump = 0;
        }
    }


    if (PlayerLock) {
        return;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        character[1]--;
    }

    if (GetAsyncKeyState('D') & 0x8000) {
        character[1]++;
    }

    if (GetAsyncKeyState('W') & 0x8000) {
        if (bounce) {
            if (doubleJump < 2) {
                doubleJump += 1;
                bounce = false;
                startJumpPos = character[0];
                playerJump = true;
                jumpInt = -10;
            }
        }
    } else {
        bounce = true;
    }
}

void playerEnvCheck() {
    int nextPlayerPos = character[0] * screenWidth + character[1];

    if (gameChar[nextPlayerPos] == '/' || gameChar[nextPlayerPos] == '\\') {
        PlayerLock = true;
        playerJump = false;
        loadMapFromFile();
    }

    if (gameChar[nextPlayerPos] == '%') {
        loadLevel++;
        PlayerLock = true;
        playerJump = false;
        loadMapFromFile();
    }
}

bool groundCheck(int nextPos) {
    string loc = render[nextPos * screenWidth + character[1]];

    if (loc == printT("#", 3) || loc == printT("=", 4)) {
        PlayerLock = false;
        return true;
    }
    return false;
}

void animateScreen(string block, int inc) {

    if (aminSpeed % 5 == 0) {
        int fire[] = {2, 4};
        int water[] = {5, 7};

        switch (gameChar[inc]) {
            case '/':
                render[inc] = printT("/", fire[rand() % 2]);
                break;
            case '\\':
                render[inc] = printT("\\", fire[rand() % 2]);
                break;
            case '~':
                render[inc] = printT("~", water[rand() % 2]);
                break;
        }
    }
}

void renderGame() {
    auto start = high_resolution_clock::now();

    int inc = 0;
    for (int y = 0; y < screenHeight; y++) {
        string allText;
        for (int x = 0; x < screenWidth; x++) {
            string staticBlock = render[inc];
            animateScreen(staticBlock, inc);

            if (y == character[0] && x == character[1]) staticBlock = printT("$", 6);
            allText += staticBlock;
            inc++;
        }
        cout << allText << "\n";
    }

    auto stop = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(stop - start);
    cout << "Time it took to render in milliseconds: " << dur.count() << " " << endl;

    if (gameWon) {
        exit(1);
    }


    for (int i = 0; i < screenHeight + 1; i++) {
        cout << "\x1b[A";
    }
}

void loadMapFromFile() {

    FILE *pF = fopen("game.txt", "r");


    string *gameMap = new string[screenHeight];
    char buffer[screenWidth + 2];
    int incrInt = 0;

    for (int i = 0; i < ((loadLevel + 1) * screenHeight); i++) {
        fgets(buffer, screenWidth + 2, pF);
        if (i >= (loadLevel * screenHeight)) {
            buffer[strcspn(buffer, "\n")] = 0;
            gameMap[incrInt] = buffer;
            incrInt++;
        }
    }

    int incR = 0;
    int chars = 0;
    for (int i = 0; i < screenHeight; i++) {
        for (int j = 0; j < screenWidth; j++) {
            gameChar[incR] = gameMap[i][j];
            string TextBlock;

            switch (gameMap[i][j]) {
                case '#':
                    TextBlock = printT("#", 3);
                    break;
                case '.':
                    TextBlock = printT(" ", 1);
                    break;
                case '=':
                    TextBlock = printT("=", 4);
                    break;
                case '%':
                    TextBlock = printT("%", 7);
                    break;
                case '/':
                    TextBlock = printT("/", 2);
                    break;
                case '\\':
                    TextBlock = printT("\\", 3);
                    break;
                case '$':
                    character[0] = i;
                    character[1] = j;
                    TextBlock = printT(".", 1);
                    break;
                case '!':
                    gameWon = true;
                    TextBlock = printT("!", 2);
                    break;
                case '-':
                    TextBlock = printT("-", 8);
                    break;
                case '>':
                    TextBlock = printT(">", 6);
                    break;
                case '<':
                    TextBlock = printT("<", 6);
                    break;

                default:
                    string B = string(1, gameMap[i][j]);
                    TextBlock = printT(B, 4);
            }

            render[incR] = TextBlock;
            incR++;
        }
    }

    fclose(pF);
}

void sleep(int milliseconds) {
    clock_t time_end;
    time_end = clock() + milliseconds * CLOCKS_PER_SEC / 1000;
    while (clock() < time_end) {
    }
}

string printT(string text, int color) {
    switch (color) {
        case 1: // gray
            text = "\e[30m" + text + "\e[0m";
        case 2: // red
            text = "\e[31m" + text + "\e[0m";
        case 3: // green
            text = "\e[32m" + text + "\e[0m";
        case 4: // yellow
            text = "\e[33m" + text + "\e[0m";
        case 5: // blue
            text = "\e[34m" + text + "\e[0m";
        case 6: // purple
            text = "\e[35m" + text + "\e[0m";
        case 7: // light blue
            text = "\e[36m" + text + "\e[0m";
        case 8: // white
            text = text;
        default:
            break;
    }
    return text;
}