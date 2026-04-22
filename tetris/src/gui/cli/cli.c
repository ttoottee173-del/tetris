// #define _POSIX_C_SOURCE 200809L // Для usleep
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "tetris.h"

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define NEXT_WIDTH 4
#define NEXT_HEIGHT 4

void print_game_info(GameInfo_t info, GameState state) {
  clear();
  if (state == STATE_START) {
    mvprintw(FIELD_HEIGHT / 2, FIELD_WIDTH, "Press SPACE to start");
  } else if (state == STATE_GAME_OVER) {
    mvprintw(FIELD_HEIGHT / 2, FIELD_WIDTH - 8,
             "Game Over! Restarting in 5 seconds...");
    mvprintw(FIELD_HEIGHT / 2 + 1, FIELD_WIDTH - 8,
             "Press SPACE to restart now");
  } else {
    // Отрисовка игрового поля
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      for (int j = 0; j < FIELD_WIDTH; j++) {
        if (info.field[i][j]) {
          attron(COLOR_PAIR(info.field[i][j]));  // Включаем цвет для блока
          mvprintw(i, j * 3, "[ ]");
          attroff(COLOR_PAIR(info.field[i][j]));  // Выключаем цвет
        } else {
          mvprintw(i, j * 3, " . ");
        }
      }
    }
    // Отрисовка следующей фигуры
    mvprintw(0, FIELD_WIDTH * 3 + 4, "Next:");
    for (int i = 0; i < NEXT_HEIGHT; i++) {
      for (int j = 0; j < NEXT_WIDTH; j++) {
        if (info.next[i][j]) {
          attron(COLOR_PAIR(info.next[i][j]));  // Включаем цвет для блока
          mvprintw(i + 1, (FIELD_WIDTH * 3 + 4) + j * 3, "[ ]");
          attroff(COLOR_PAIR(info.next[i][j]));  // Выключаем цвет
        } else {
          mvprintw(i + 1, (FIELD_WIDTH * 3 + 4) + j * 3, " . ");
        }
      }
    }
    // Боковая панель
    attron(COLOR_PAIR(1));  // Красный для Score
    mvprintw(6, FIELD_WIDTH * 3 + 4, "Score: %d", info.score);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));  // Красный для Score
    mvprintw(7, FIELD_WIDTH * 3 + 4, "High: %d", info.high_score);
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(3));  // Красный для Score
    mvprintw(8, FIELD_WIDTH * 3 + 4, "Level: %d", info.level);
    attroff(COLOR_PAIR(3));
    attron(COLOR_PAIR(4));  // Красный для Score
    mvprintw(9, FIELD_WIDTH * 3 + 4, "Speed: %d", info.speed);
    attroff(COLOR_PAIR(4));
    attron(COLOR_PAIR(5));  // Красный для Score
    mvprintw(10, FIELD_WIDTH * 3 + 4, "Pause: %s", info.pause ? "Yes" : "No");
    attroff(COLOR_PAIR(5));
    // отладка
    mvprintw(21, 0, "Debug: State=%d", state);
  }

  refresh();
}

int main() {
  srand(time(NULL));
  TetrisGame game;
  init_game(&game);

  initscr();
  start_color();
  // Инициализация цветных пар (1-6)
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  timeout(10);

  bool running = true;
  bool hold = false;
  long last_input_time = get_time_ms();
  const long HOLD_DELAY = 500;

  while (running) {
    GameInfo_t info = updateCurrentState(&game);
    print_game_info(info, game.state);

    int ch = getch();
    UserAction_t action = Terminate;

    if (ch != ERR) {
      long current_time = get_time_ms();
      long elapsed = current_time - last_input_time;

      switch (ch) {
        case ' ':
          action = (game.state == STATE_START || game.state == STATE_GAME_OVER)
                       ? Start
                       : Drop;
          break;
        case 'p':
          action = Pause;
          break;
        case 'q':
          action = Terminate;
          break;
        case KEY_LEFT:
          action = Left;
          break;
        case KEY_RIGHT:
          action = Right;
          break;
        case KEY_DOWN:
          action = Down;
          break;
        case KEY_UP:
          action = Action;
          break;  // Вращение на KEY_UP
        case 'w':
          action = Action;
          break;  // Вращение на w
        case 's':
          action = Down;
          break;
        case 'a':
          action = Left;
          break;
        case 'd':
          action = Right;
          break;
      }

      if (action == Left || action == Right || action == Down) {
        if (elapsed >= HOLD_DELAY) {
          hold = true;
          fprintf(stderr, "Input: Hold activated, action=%d, elapsed=%ld\n",
                  action, elapsed);
        } else {
          hold = false;
          fprintf(stderr, "Input: Single press, action=%d, elapsed=%ld\n",
                  action, elapsed);
        }
      } else {
        hold = false;
        fprintf(stderr, "Input: Action=%d, hold reset\n", action);
      }

      userInput(&game, action, hold);
      last_input_time = current_time;

      if (action == Terminate) {
        running = false;
      }
    }

    for (int i = 0; i < FIELD_HEIGHT; i++) free(info.field[i]);
    for (int i = 0; i < NEXT_HEIGHT; i++) free(info.next[i]);
    free(info.field);
    free(info.next);
  }

  endwin();
  return 0;
}
