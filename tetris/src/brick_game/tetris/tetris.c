#include "tetris.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

long get_time_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

static int read_high_score(void) {
  FILE* file = fopen("high_score.txt", "r");
  int high_score = 0;
  if (file) {
    fscanf(file, "%d", &high_score);
    fclose(file);
  }
  return high_score;
}

static void write_high_score(int high_score) {
  FILE* file = fopen("high_score.txt", "w");
  if (file) {
    fprintf(file, "%d", high_score);
    fclose(file);
  }
}

void init_game(TetrisGame* game) {
  game->state = STATE_START;
  game->game_active = false;  // Игра не активна до нажатия пробела
  memset(game->matrix, 0, sizeof(int) * 20 * 10);
  game->current_tetromino = rand() % 7;
  game->current_rotation = 0;
  memcpy(game->current_piece,
         tetrominoes[game->current_tetromino][game->current_rotation],
         sizeof(int) * 4 * 4);
  game->next_tetromino = rand() % 7;
  game->next_rotation = 0;
  memcpy(game->next_piece,
         tetrominoes[game->next_tetromino][game->next_rotation],
         sizeof(int) * 4 * 4);
  game->piece_x = 3;
  game->piece_y = 0;
  game->last_update = get_time_ms();
  game->level = 1;  // Устанавливаем level перед вызовом lvl_speed
  game->drop_speed_ms = lvl_speed(game);
  game->score = 0;
  game->lines_cleared = 0;
  game->last_move_time = 0;
  game->game_over_start_time = 0;
  game->current_color = rand() % 6 + 1;  // Случайный цвет (1-6)
  game->next_color =
      (game->current_color % 6) + 1;  // Разный цвет для следующей фигуры
}

bool can_drop(const TetrisGame* game) {
  int new_y = game->piece_y + 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game->current_piece[i][j]) {
        int new_matrix_y = new_y + i;
        int matrix_x = game->piece_x + j;
        if (new_matrix_y >= 20 ||
            (new_matrix_y >= 0 && matrix_x >= 0 && matrix_x < 10 &&
             game->matrix[new_matrix_y][matrix_x])) {
          return false;
        }
      }
    }
  }
  return true;
}

bool can_spawn(const TetrisGame* game) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game->current_piece[i][j]) {
        int matrix_y = game->piece_y + i;
        int matrix_x = game->piece_x + j;
        if (matrix_y >= 0 && matrix_y < 20 && matrix_x >= 0 && matrix_x < 10 &&
            game->matrix[matrix_y][matrix_x]) {
          return false;
        }
      }
    }
  }
  return true;
}

bool can_rotate(TetrisGame* game) {
  int new_rotation = (game->current_rotation + 1) % 4;
  int rotated[4][4];
  memcpy(rotated, tetrominoes[game->current_tetromino][new_rotation],
         sizeof(int) * 4 * 4);
  bool collision = false;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (rotated[i][j]) {
        int matrix_y = game->piece_y + i;
        int matrix_x = game->piece_x + j;
        // cppcheck-suppress knownConditionTrueFalse
        if (matrix_y >= 20 || matrix_x >= 10 || matrix_x < 0 ||
            (matrix_y >= 0 && matrix_x >= 0 && matrix_x < 10 &&
             game->matrix[matrix_y][matrix_x])) {
          collision = true;
          break;
        }
      }
    }
    if (collision) break;
  }
  if (!collision) {
    game->current_rotation = new_rotation;
    memcpy(game->current_piece, rotated, sizeof(int) * 4 * 4);

    return true;
  }
  return false;
}

bool can_move(const TetrisGame* game, int dx) {
  int new_x = game->piece_x + dx;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game->current_piece[i][j]) {
        int matrix_y = game->piece_y + i;
        int matrix_x = new_x + j;
        // cppcheck-suppress knownConditionTrueFalse
        if (matrix_y >= 20 || matrix_x >= 10 || matrix_x < 0 ||
            (matrix_y >= 0 && matrix_x >= 0 && matrix_x < 10 &&
             game->matrix[matrix_y][matrix_x])) {
          return false;
        }
      }
    }
  }
  return true;
}

void clear_lines(TetrisGame* game) {
  int lines = 0;
  for (int i = 0; i < 20; i++) {
    int full = 1;
    for (int j = 0; j < 10; j++) {
      if (!game->matrix[i][j]) {
        full = 0;
        break;
      }
    }
    if (full) {
      lines++;
      for (int k = i; k > 0; k--) {
        for (int j = 0; j < 10; j++) {
          game->matrix[k][j] = game->matrix[k - 1][j];
        }
      }
      memset(game->matrix[0], 0, sizeof(int) * 10);
      i--;  // Повторная проверка строки после сдвига
    }
  }
  switch (lines) {
    case 1:
      game->score += 100;
      break;
    case 2:
      game->score += 300;
      break;
    case 3:
      game->score += 700;
      break;
    case 4:
      game->score += 1500;
      break;
  }
  game->lines_cleared += lines;
  game->level = 1 + (game->score / 600);
  game->drop_speed_ms = lvl_speed(game);
}

int lvl_speed(const TetrisGame* game) {
  int speed_lvl = 1000;
  if (game->level < 11) {
    for (int i = 1; i < game->level; i++) {
      speed_lvl -= speed_lvl / 5;
    }
  }
  return speed_lvl > 50 ? speed_lvl : 50;
}

void userInput(TetrisGame* game, UserAction_t action, bool hold) {
  long current_time = get_time_ms();
  long elapsed_move = current_time - game->last_move_time;

  if (game->state == STATE_START && action != Start) return;
  if (game->state == STATE_GAME_OVER && action != Start) return;
  if (game->state == STATE_PAUSE && action != Pause && action != Terminate)
    return;

  switch (action) {
    case Start:
      if (game->state == STATE_START || game->state == STATE_GAME_OVER) {
        init_game(game);
        game->state = STATE_SPAWN;
        game->game_active = true;
      }
      break;
    case Pause:
      if (game->game_active && game->state != STATE_GAME_OVER) {
        game->state = (game->state == STATE_PAUSE) ? STATE_MOVING : STATE_PAUSE;
      }
      break;
    case Terminate:
      game->game_active = false;
      break;
    case Left:
      if ((hold && elapsed_move >= MOVE_DELAY) ||
          (!hold && can_move(game, -1))) {
        if (can_move(game, -1)) {
          game->piece_x--;
          game->last_move_time = current_time;
        }
      }
      break;
    case Right:
      if ((hold && elapsed_move >= MOVE_DELAY) ||
          (!hold && can_move(game, 1))) {
        if (can_move(game, 1)) {
          game->piece_x++;
          game->last_move_time = current_time;
        }
      }
      break;
    case Down:
      if (can_drop(game)) {
        game->piece_y++;
        game->last_update = current_time;
      }
      break;
    case Drop:
      if (game->state == STATE_MOVING || game->state == STATE_SHIFTING) {
        if (can_drop(game)) {
          while (can_drop(game)) {
            game->piece_y++;
          }
          game->last_update = current_time;
          game->state = STATE_ATTACHING;
        }
      }
      break;
    case Action:
      if (game->state == STATE_MOVING || game->state == STATE_SHIFTING) {
        can_rotate(game);
      }
      break;
    case Up:
      break;
  }
}

GameInfo_t updateCurrentState(TetrisGame* game) {
  GameInfo_t info;
  info.field = malloc(20 * sizeof(int*));
  info.next = malloc(4 * sizeof(int*));
  for (int i = 0; i < 20; i++) info.field[i] = malloc(10 * sizeof(int));
  for (int i = 0; i < 4; i++) info.next[i] = malloc(4 * sizeof(int));

  // Копируем matrix в info.field
  for (int i = 0; i < 20; i++)
    for (int j = 0; j < 10; j++) info.field[i][j] = game->matrix[i][j];

  // Добавляем текущую фигуру в info.field для STATE_MOVING, STATE_SHIFTING и
  // STATE_PAUSE
  if (game->state == STATE_MOVING || game->state == STATE_SHIFTING ||
      game->state == STATE_PAUSE) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if (game->current_piece[i][j]) {
          int matrix_y = game->piece_y + i;
          int matrix_x = game->piece_x + j;
          if (matrix_y >= 0 && matrix_y < 20 && matrix_x >= 0 &&
              matrix_x < 10) {
            info.field[matrix_y][matrix_x] =
                game->current_color;  // Используем цвет текущей фигуры
          }
        }
      }
    }
  }

  // Копируем следующую фигуру
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      info.next[i][j] = game->next_piece[i][j]
                            ? game->next_color
                            : 0;  // Цвет для следующей фигуры

  int high_score = read_high_score();
  info.score = game->score;
  if (game->score > high_score) {
    high_score = game->score;
    write_high_score(high_score);
  }
  info.high_score = high_score;
  info.level = game->level;
  info.speed = game->drop_speed_ms;
  info.pause = (game->state == STATE_PAUSE) ? 1 : 0;

  if (game->game_active && game->state != STATE_PAUSE &&
      game->state != STATE_START) {
    long current_time = get_time_ms();
    long elapsed_ms = current_time - game->last_update;
    switch (game->state) {
      case STATE_SPAWN:
        game->current_tetromino = game->next_tetromino;
        game->current_rotation = game->next_rotation;
        game->current_color =
            game->next_color;  // Переносим цвет следующей фигуры
        memcpy(game->current_piece, game->next_piece, sizeof(int) * 4 * 4);
        game->next_tetromino = rand() % 7;
        game->next_rotation = 0;
        do {
          game->next_color = rand() % 6 + 1;
        } while (game->next_color == game->current_color);
        memcpy(game->next_piece,
               tetrominoes[game->next_tetromino][game->next_rotation],
               sizeof(int) * 4 * 4);
        game->piece_x = 3;
        game->piece_y = 0;
        if (!can_spawn(game)) {
          game->state = STATE_GAME_OVER;
          game->game_over_start_time = current_time;
        } else {
          game->state = STATE_MOVING;
        }
        game->last_update = current_time;
        break;
      case STATE_MOVING:
        if (elapsed_ms >= game->drop_speed_ms) {
          if (can_drop(game)) {
            game->state = STATE_SHIFTING;
          } else {
            game->state = STATE_ATTACHING;
          }
          game->last_update = current_time;
        }
        break;
      case STATE_SHIFTING:
        if (can_drop(game)) {
          game->piece_y++;
          game->state = STATE_MOVING;
          game->last_update = current_time;
        } else {
          game->state = STATE_ATTACHING;
        }
        break;
      case STATE_ATTACHING:
        for (int i = 0; i < 4; i++)
          for (int j = 0; j < 4; j++)
            if (game->current_piece[i][j]) {
              int new_y = game->piece_y + i;
              int new_x = game->piece_x + j;
              if (new_y >= 0 && new_y < 20 && new_x >= 0 && new_x < 10) {
                game->matrix[new_y][new_x] =
                    game->current_color;  // Сохраняем цвет фигуры
              }
            }
        clear_lines(game);
        game->state = STATE_SPAWN;
        game->last_update = current_time;
        break;
      case STATE_GAME_OVER:
        if (current_time - game->game_over_start_time >= 5000) {
          init_game(game);
          game->state = STATE_START;
        }
        break;
      case STATE_START:
      case STATE_PAUSE:
        break;
    }
  }

  return info;
}