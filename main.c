#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define STARTING_SEGMENTS 3

typedef enum {
  RIGHT,
  DOWN,
  LEFT,
  UP
} direction_t;

typedef struct body_segment {
  int x;
  int y;
  struct body_segment *next;
} body_segment_t;

typedef struct {
  direction_t direction;
  int length;
  body_segment_t *head;
} player_t;

void death(int length)
{
  clear();
  mvaddstr(0, 0, "YOU DIED");
  mvprintw(1, 0, "Score: %d", length - STARTING_SEGMENTS);

  refresh();
  sleep(1);
  endwin();
  refresh();
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
  int max_y;
  int max_x;
  int ch = ERR;
  player_t player;
  int x_idx;
  int y_idx;
  int add_one = 0;
  body_segment_t *body_ptr;
  int next_x;
  int next_y;
  int prev_x;
  int prev_y;
  int fruit_x;
  int fruit_y;
  int i;

  initscr();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  getmaxyx(stdscr, max_y, max_x);

  srand(time(NULL));

  for (x_idx = 0; x_idx < max_x; x_idx++) {
    mvaddstr(0, x_idx, "#");
    mvaddstr(max_y - 1, x_idx, "#");
  }
  for (y_idx = 1; y_idx < max_y - 1; y_idx++) {
    mvaddstr(y_idx, 0, "#");
    mvaddstr(y_idx, max_x - 1, "#");
  }

  player.length = STARTING_SEGMENTS;
  player.direction = RIGHT;
  player.head = (body_segment_t *) malloc(sizeof(body_segment_t));
  body_ptr = player.head;
  for (i = 0; i < STARTING_SEGMENTS; i++) {
    body_ptr->x = max_x / 2 - i;
    body_ptr->y = max_y / 2;

    if (i < STARTING_SEGMENTS - 1) {
      body_ptr->next = (body_segment_t *) malloc(sizeof(body_segment_t));
    }
    else {
      body_ptr->next = 0;
    }

    body_ptr = body_ptr->next;
  }

  body_ptr = player.head;

  fruit_x = rand() % (max_x - 2) + 1;
  fruit_y = rand() % (max_y - 2) + 1;
  mvaddstr(fruit_y, fruit_x, "@");

  while(1) {
    while ((ch = getch()) != ERR) {
      switch (ch) {
        case KEY_UP:
          player.direction = UP;
          break;
        case KEY_DOWN:
          player.direction = DOWN;
          break;
        case KEY_LEFT:
          player.direction = LEFT;
          break;
        case KEY_RIGHT:
          player.direction = RIGHT;
          break;
      }
    }

    prev_x = player.head->x;
    prev_y = player.head->y;

    switch (player.direction) {
      case UP:
        prev_y -= 1;
        break;
      case DOWN:
        prev_y += 1;
        break;
      case LEFT:
        prev_x -= 1;
        break;
      case RIGHT:
        prev_x += 1;
        break;
    }

    if (prev_y == 0 || prev_x == 0 ||
        prev_y == max_y - 1 || prev_x == max_x - 1) {
      death(player.length);
    }
    else if (prev_y == fruit_y && prev_x == fruit_x) {
      add_one = 1;
      player.length += 1;
      fruit_x = rand() % (max_x - 2) + 1;
      fruit_y = rand() % (max_y - 2) + 1;
      mvaddstr(fruit_y, fruit_x, "@");
    }

    body_ptr = player.head;

    while (body_ptr) {
      next_x = body_ptr->x;
      next_y = body_ptr->y;

      body_ptr->x = prev_x;
      body_ptr->y = prev_y;

      if (body_ptr != player.head &&
          body_ptr->x == player.head->x &&
          body_ptr->y == player.head->y) {
        death(player.length);
      }

      prev_x = next_x;
      prev_y = next_y;

      mvaddstr(body_ptr->y, body_ptr->x, "%");

      if (!body_ptr->next && add_one) {
        body_ptr->next = (body_segment_t *) malloc(sizeof(body_segment_t));
        body_ptr->next->x = prev_x;
        body_ptr->next->y = prev_y;
        body_ptr->next->next = 0;
        add_one = 0;
      }

      body_ptr = body_ptr->next;
    }

    if (player.head->x != prev_x || player.head->y != prev_y) {
      mvaddstr(prev_y, prev_x, " ");
    }

    refresh();
    if (player.direction == UP || player.direction == DOWN) {
      usleep(150000);
    }
    else {
      usleep(100000);
    }
  }

  endwin(); 
  refresh();

  return 0;
}

