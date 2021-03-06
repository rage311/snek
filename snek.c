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


void death(int length, body_segment_t *head)
{
  body_segment_t *tmp_ptr;

  clear();
  mvaddstr(0, 0, "YOU DIED");
  mvprintw(1, 0, "Score: %d\n", length - STARTING_SEGMENTS);

  refresh();
  sleep(1);
  endwin();
  refresh();
  curs_set(TRUE);

  while (head) {
    tmp_ptr = head;
    head = head->next;
    free(tmp_ptr);
  }
  tmp_ptr = NULL;
}

void place_fruit(int *fruit_xy, body_segment_t *head, int max_x, int max_y)
{
  int fruit_x,
      fruit_y;

  body_segment_t *body_ptr;

  fruit_x = rand() % (max_x - 2) + 1;
  fruit_y = rand() % (max_y - 3) + 2;

  body_ptr = head;

  /* Ensure fruit doesn't get placed on snake */
  while (body_ptr) {
    if (fruit_x == body_ptr->x && fruit_y == body_ptr->y) {
      fruit_x = rand() % (max_x - 2) + 1;
      fruit_y = rand() % (max_y - 3) + 2;
      body_ptr = head;
    } else {
      body_ptr = body_ptr->next;
    }
  }

  /* Draw fruit */
  mvaddstr(fruit_y, fruit_x, "@");

  /* Copy values for "returned" array */
  fruit_xy[0] = fruit_x;
  fruit_xy[1] = fruit_y;

  body_ptr = NULL;
}

int main(int argc, char **argv)
{
  int max_y;
  int max_x;
  int ch = ERR;
  player_t player;
  int x_idx;
  int y_idx;
  unsigned char add_one = 0;
  body_segment_t *body_ptr;
  int next_x;
  int next_y;
  int prev_x;
  int prev_y;
  int fruit_xy[2];
  int i;
  char score[5] = { '0', '\0', '\0', '\0', '\0' };

  /* Initialize curses and screen */
  initscr();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  getmaxyx(stdscr, max_y, max_x);

  /* Seed rand */
  srand(time(NULL));

  /* Draw walls */
  for (x_idx = 0; x_idx < max_x; x_idx++) {
    mvaddstr(1, x_idx, "#");
    mvaddstr(max_y - 1, x_idx, "#");
  }
  for (y_idx = 1; y_idx < max_y - 1; y_idx++) {
    mvaddstr(y_idx, 0, "#");
    mvaddstr(y_idx, max_x - 1, "#");
  }

  mvaddstr(0, 0, "Score: 0");

  /* Initial snake values */
  player.length = STARTING_SEGMENTS;
  player.direction = RIGHT;
  player.head = (body_segment_t *) malloc(sizeof(body_segment_t));

  body_ptr = player.head;
  for (i = 0; i < STARTING_SEGMENTS; i++) {
    body_ptr->x = max_x / 2 - i;
    body_ptr->y = max_y / 2;

    if (i < STARTING_SEGMENTS - 1) {
      body_ptr->next = (body_segment_t *) malloc(sizeof(body_segment_t));
    } else {
      body_ptr->next = 0;
    }

    body_ptr = body_ptr->next;
  }

  body_ptr = player.head;

  place_fruit(fruit_xy, player.head, max_x, max_y);

  while (1) {
    prev_x = player.head->x;
    prev_y = player.head->y;

    if ((ch = getch()) != ERR) {
      switch (ch) {
        case KEY_UP:
        case 'k':
          player.direction = UP;
          break;
        case KEY_DOWN:
        case 'j':
          player.direction = DOWN;
          break;
        case KEY_LEFT:
        case 'h':
          player.direction = LEFT;
          break;
        case KEY_RIGHT:
        case 'l':
          player.direction = RIGHT;
          break;
      }
    }

    switch (player.direction) {
      case UP:
        --prev_y;
        break;
      case DOWN:
        ++prev_y;
        break;
      case LEFT:
        --prev_x;
        break;
      case RIGHT:
        ++prev_x;
        break;
    }

    /* Check if we've hit a wall */
    if (prev_y <= 1 || prev_x == 0 ||
        prev_y == max_y - 1 || prev_x == max_x - 1) {
      death(player.length, player.head);
      player.head = NULL;
      body_ptr = NULL;
      exit(EXIT_SUCCESS);
    } else if (prev_y == fruit_xy[1] && prev_x == fruit_xy[0]) {
      add_one = 1;
    }

    body_ptr = player.head;

    while (body_ptr) {
      next_x = body_ptr->x;
      next_y = body_ptr->y;

      body_ptr->x = prev_x;
      body_ptr->y = prev_y;

      /* If the head runs into the body */
      if (body_ptr != player.head &&
          body_ptr->x == player.head->x &&
          body_ptr->y == player.head->y) {
        death(player.length, player.head);
        player.head = NULL;
        body_ptr = NULL;
        exit(EXIT_SUCCESS);
      }

      prev_x = next_x;
      prev_y = next_y;

      /* Draw current position as part of the snake */
      mvaddstr(body_ptr->y, body_ptr->x, "%");

      /* If we're at the end of the tail and need to extend the length */
      if (!body_ptr->next && add_one) {
        body_ptr->next = (body_segment_t *) malloc(sizeof(body_segment_t));
        body_ptr->next->x = prev_x;
        body_ptr->next->y = prev_y;
        body_ptr->next->next = 0;

        ++player.length;
        snprintf(score, 5, "%d", player.length - STARTING_SEGMENTS);
        mvaddstr(0, 7, score);
        place_fruit(fruit_xy, player.head, max_x, max_y);
        add_one = 0;
      }

      body_ptr = body_ptr->next;
    }

    /* Blank out previous tail end position unless that's where the new
     * fruit is or where the head is moving to */
    if ((player.head->x != prev_x || player.head->y != prev_y) &&
        (prev_y != fruit_xy[1] || prev_x != fruit_xy[0])) {
      mvaddstr(prev_y, prev_x, " ");
    }

    refresh();

    /* Sleep before next cycle
     * Sleep longer if going up or down, since terminal characters are taller
     * than they are wide and we want the speed to feel the same */
    if (player.direction == UP || player.direction == DOWN) {
      usleep(160000);
    } else {
      usleep(100000);
    }
  }

  return 0;
}

