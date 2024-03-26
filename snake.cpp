#include "console.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#endif
using namespace std;
#define BOARD_WIDTH 15
#define BOARD_HEIGHT 15
#define SNAKE_MAX_LENGTH (BOARD_WIDTH - 2) * (BOARD_HEIGHT - 2)
#define MOVE_DELAY 15
#define WALL_VERTICAL_STRING u8"┃"
#define WALL_HORIZONTAL_STRING u8"━"
#define WALL_RIGHT_TOP_STRING u8"┓"
#define WALL_LEFT_TOP_STRING u8"┏"
#define WALL_RIGHT_BOTTOM_STRING u8"┛"
#define WALL_LEFT_BOTTOM_STRING u8"┗"
#define SNAKE_STRING u8"■"
#define SNAKE_BODY_STRING u8"■"
#define APPLE_STRING u8"●"

int apx; // 사과 x 좌표
int apy; // 사과 y 좌표
int snake_len = 1; // 뱀길이
int score = 0; // 점수
int snakeX[SNAKE_MAX_LENGTH]; // 뱀 머리와 몸통의 X좌표를 저장할 배열
int snakeY[SNAKE_MAX_LENGTH]; // 뱀 머리와 몸통의 Y좌표를 저장할 배열
bool game_over = false; // 게임패배 여부 확인
console::Key newDir = console::K_LEFT; // 입력받은 키를 저장
void Score();
///////////////////////////////////////////////////////////////////////////////////
// 보드판을 그림
void Board() {
    // 가로선 그리기
    for (int i = 0; i < BOARD_WIDTH; ++i) {
        console::draw(i, 0, WALL_HORIZONTAL_STRING);
        console::draw(i, BOARD_WIDTH - 1, WALL_HORIZONTAL_STRING);
    }

    // 세로선 그리기
    for (int j = 1; j < BOARD_HEIGHT - 1; ++j) {
        console::draw(0, j, WALL_VERTICAL_STRING);
        console::draw(BOARD_HEIGHT - 1, j, WALL_VERTICAL_STRING);
    }

    // 모서리 그리기
    console::draw(0, 0, WALL_LEFT_TOP_STRING);
   
    console::draw(BOARD_WIDTH - 1, 0, WALL_RIGHT_TOP_STRING);
    
    console::draw(0, BOARD_WIDTH - 1, WALL_LEFT_BOTTOM_STRING);
    
    console::draw(BOARD_WIDTH - 1, BOARD_HEIGHT - 1, WALL_RIGHT_BOTTOM_STRING);

    Score();
}


///////////////////////////////////////////////////////////////////////////////////
// 키를 입력받아 뱀의 머리가 가는 방향을 정함
void drive() {
    // 뱀의 길이가 1일때는 가는 방향과 반대키를 눌러도 인식됨
    if (snake_len == 1) {
        if (console::key(console::K_UP)) {
            newDir = console::K_UP;
        }
        else if (console::key(console::K_DOWN)) {
            newDir = console::K_DOWN;
        }
        else if (console::key(console::K_LEFT)) {
            newDir = console::K_LEFT;
        }
        else if (console::key(console::K_RIGHT)) {
            newDir = console::K_RIGHT;
        }
    }
    // 뱀의 길이가 2 이상일때 새로 키를 누르면 이전 키의 방향과 새로운 키의 방향이 달라야 인식됨
    else if (snake_len > 1) {
        if (console::key(console::K_UP) && newDir != console::K_DOWN) {
            newDir = console::K_UP;
        }
        else if (console::key(console::K_DOWN) && newDir != console::K_UP) {
            newDir = console::K_DOWN;
        }
        else if (console::key(console::K_LEFT) && newDir != console::K_RIGHT) {
            newDir = console::K_LEFT;
        }
        else if (console::key(console::K_RIGHT) && newDir != console::K_LEFT) {
            newDir = console::K_RIGHT;
        }
    }
}

void drawApple();
void Restart_Or_Out();
///////////////////////////////////////////////////////////////////////////////////
// 뱀의 머리, 몸통의 위치를 조정하고, 뱀의 머리 위치에따라 게임결과 여부를 정함.
void moveSnake() {
    if (!game_over) { // 게임 오버 상태가 아닌 경우에만 뱀의 움직임 처리
        // 사과를 먹었을때 길이가 늘어나므로, 머리를 움직이기전에 뱀 몸통들을 뒤로 밀어냄
        for (int i = snake_len - 1; i > 0; i--) {
            snakeX[i] = snakeX[i - 1];
            snakeY[i] = snakeY[i - 1];
        }

        // 뱀의 머리를 움직임
        if (newDir == console::K_UP) {
            snakeY[0]--;
        }
        else if (newDir == console::K_DOWN) {
            snakeY[0]++;
        }
        else if (newDir == console::K_LEFT) {
            snakeX[0]--;
        }
        else if (newDir == console::K_RIGHT) {
            snakeX[0]++;
        }

        // 벽에 박으면 gameover
        if (snakeX[0] <= 0 || snakeX[0] >= BOARD_WIDTH - 1 || snakeY[0] <= 0 || snakeY[0] >= BOARD_HEIGHT - 1) {
            game_over = true;
        }

        // 뱀의 몸통에 박으면 gameover
        for (int i = 1; i < snake_len; i++) {
            if (snakeX[i] == snakeX[0] && snakeY[i] == snakeY[0]) {
                game_over = true;
            }
        }
        
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 뱀을 그림
void drawSnake() {
    for (int i = 0; i < snake_len; i++) {
        console::draw(snakeX[i], snakeY[i], SNAKE_BODY_STRING);
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 사과를 그림
void drawApple() {
    console::draw(apx, apy, APPLE_STRING);
}

///////////////////////////////////////////////////////////////////////////////////
void makeApple() {
    // 뱀이 없는 위치의 좌표를 저장하는 배열 초기화
    int availableAppleX[SNAKE_MAX_LENGTH * SNAKE_MAX_LENGTH];
    int availableAppleY[SNAKE_MAX_LENGTH * SNAKE_MAX_LENGTH];
    int availableCount = 0;

    // 보드 내의 모든 위치에 대해 확인하여 뱀의 몸통이 아닌 위치를 찾음
    for (int i = 1; i < BOARD_WIDTH - 1; i++) {
        for (int j = 1; j < BOARD_HEIGHT - 1; j++) {
            bool isSnakeBody = false;
            // 해당 위치가 뱀의 몸통인지 확인
            for (int k = 0; k < snake_len; ++k) {
                if (snakeX[k] == i && snakeY[k] == j) {
                    isSnakeBody = true;
                    break;
                }
            }
            // 현재 위치가 뱀의 몸통이 아니라면 가능한 사과 위치로 저장
            if (!isSnakeBody) {
                availableAppleX[availableCount] = i;
                availableAppleY[availableCount] = j;
                availableCount++;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // 뱀이 없는 위치가 하나라도 있는 경우에만 사과를 그림
    if (availableCount > 0) {
        // 랜덤하게 사과의 위치를 선택
        int index = rand() % availableCount;
        apx = availableAppleX[index];
        apy = availableAppleY[index];
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 뱀이 사과를 먹었을 때
void eatApple() {
    if (snakeX[0] == apx && snakeY[0] == apy) {
        snake_len++;
        score += 10;
        makeApple(); // 뱀이 사과를 먹었을 때만 사과를 다시 그림
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 점수를 나타냄
void Score() {
    string scoreboard = "Score: " + to_string(score);
    int scoreX = (BOARD_WIDTH - scoreboard.length()) / 2; // 점수 표시 위치 계산
    console::draw(scoreX, BOARD_HEIGHT + 1, scoreboard); // 맵 바로 아래 중앙에 점수 표시
}

///////////////////////////////////////////////////////////////////////////////////
// 승리 여부 확인
bool win() {
    if (snake_len == SNAKE_MAX_LENGTH) {
        console::draw(BOARD_WIDTH / 2 - 4, BOARD_HEIGHT / 2, "You win!");
        console::draw(BOARD_WIDTH / 2 - 7, BOARD_HEIGHT / 2 + 1, "Try again?(Enter)");
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
// 패배 여부 확인
bool lose() {
    if (game_over) {
        console::draw(BOARD_WIDTH / 2 - 4, BOARD_HEIGHT / 2 - 1, "You lose!");
        console::draw(BOARD_WIDTH / 2 - 7, BOARD_HEIGHT / 2, "Try again?(Enter)");
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
// enter키를 누르면 게임을 다시 시작하고 esc키를 누르면 종료
void Restart_Or_Out() {
    if (console::key(console::K_ENTER)) {
        snakeX[0] = BOARD_WIDTH / 2;
        snakeY[0] = BOARD_HEIGHT / 2;
        snake_len = 1;
        makeApple();
        score = 0;
        game_over = false;
    }
    else if (console::key(console::K_ESC)) {
        exit(0);
    }
}

///////////////////////////////////////////////////////////////////////////////////
int main() {
    console::init();
    srand(time(NULL));

    snakeX[0] = BOARD_WIDTH / 2; // 뱀의 처음위치
    snakeY[0] = BOARD_HEIGHT / 2; // 뱀의 처음위치
    makeApple();    
    while (true) {
        if (!game_over) {
            drive();     // // 키 입력 처리
            moveSnake(); // 뱀 이동 
            eatApple();  // 사과 먹었는지 여부 확인
        }
        console::clear();
        Board();     // 보드 그리기
        drawSnake(); // 뱀 그리기
        drawApple(); // 사과 그리기
       

        // 승리 또는 패배 확인 후 게임 진행, 종료 처리
        if (win()) {
            Restart_Or_Out();
        }
        else if (lose()) {
            Restart_Or_Out();
        }
        for (int i = 0; i < MOVE_DELAY; ++i) {
            console::wait();
        }

    }
    return 0;
}



