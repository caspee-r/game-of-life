/*
Grid:
The game is played on a two-dimensional grid of square cells.
Each cell can be either alive (populated) or dead (unpopulated).

Neighborhood:
Each cell has eight neighbors (horizontally, vertically, and diagonally adjacent
cells).

Rules for Birth:
A dead cell with exactly three live neighbors becomes a live cell in the next
generation. This simulates reproduction.

Rules for Survival:
A live cell with two or three live neighbors survives to the next generation.

Rules for Death:
A live cell with fewer than two live neighbors dies (underpopulation).
A live cell with more than three live neighbors dies (overpopulation).

Generation Update:
All cells are updated simultaneously in each generation.
*/

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ALIVE_CELL_CHAR 'O'
#define DEAD_CELL_CHAR '.'
#define FPS 60
#define STATUS_Y 1


typedef enum {
	DEAD = 0,
	ALIVE,
} State;

typedef enum {
	TOGGLEPAUSE = ' ',
	QUIT = 'q',
	RESTART = 'r',
	NEXT = 'n',
} Actions;

typedef struct {
	State state;
} Cell;

void random_init(const int w, const int h, Cell grid[h][w]) {
	int i, j;
	srand(time(NULL));
	for (i = 1; i < h; i = i + 2) {
		for (j = 0; j < w; j++) {
			grid[i][j].state = rand() % 2;
		}
	}
}

/*
 * {0,0,1,0,0,1,1,0}
 * {0,0,1,0,1,0,1,0}
 * {0,0,0,1,1,0,1,0}
 * {0,0,0,0,0,1,1,0}
 * {0,0,0,0,0,0,1,1}
 * {1,0,1,0,0,0,0,0}
 * {0,0,0,0,0,0,1,0}
 * {0,0,1,1,0,0,0,1}
 * {1,0,1,0,0,1,0,0}
 * {0,1,0,0,0,1,1,0}
 *
 */

void next_gen(const int w, const int h, Cell grid[h][w]) {
	Cell next_grid[h][w];
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int alive_counter = 0;
			next_grid[i][j] = grid[i][j];
			for (int k = -1; k <= 1; k++) {
				for (int l = -1; l <= 1; l++) {
					if (k == 0 && l == 0)
						continue; // cheking the main cell, but we want to check
					// the neighbors
					int row = (i + k) % h;
					int col = (j + l) % w;
					if (grid[row][col].state == ALIVE) {
						alive_counter++;
					}
				}
			}
			switch (alive_counter) {
				case 0:
				case 1:
					next_grid[i][j].state = DEAD;
					break;
				case 2:
					break;
				case 3:
					next_grid[i][j].state = ALIVE;
					break;
				default:
					next_grid[i][j].state = DEAD;
					break;
			}
		}
	}
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			grid[i][j] = next_grid[i][j];
		}
	}
}

void display(WINDOW* win, const int width, const int height,
			 const Cell grid[height][width]) {
	// size_t padd = (*row - h)/ 2;
	// wprintw(win,"\n");
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (grid[i][j].state == ALIVE) {
				wprintw(win, "%c", ALIVE_CELL_CHAR);
			} else {
				wprintw(win, "%c", DEAD_CELL_CHAR);
			}
		}
		wprintw(win, "\n");
	}
	box(win, 0, 0);
	wrefresh(win);
}

void reset_grid(const int width, const int height, Cell grid[height][width]) {
	Cell dead = {0};
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			grid[i][j] = dead;
		}
	}
}

inline void display_status(int x ,int y, unsigned gen){
	attron(A_BOLD | COLOR_PAIR(1));
	mvprintw(y, x, "Generation:%d", gen);
	attroff(A_BOLD | COLOR_PAIR(1));
	refresh();
}

int main() {
	WINDOW* cwin;
	int wstartx, wstarty, wwidth, wheight;
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	curs_set(0);
	start_color();
	keypad(stdscr, TRUE);
	wheight = LINES - 4;
	wwidth = COLS / 2;
	wstarty = (LINES - wheight) / 2;
	wstartx = (COLS - wwidth) / 2; /* of the window		*/
	int gh = wheight - 1;
	int gw = wwidth - 1;
	unsigned gen = 0;
	Cell grid[gh][gw];
	cwin = newwin(wheight, wwidth, wstarty, wstartx);
	random_init(gw, gh, grid);
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	refresh();
	int ch, is_paused;
	is_paused = 0;
	while ((ch = getch()) != QUIT) {
		switch (ch) {
			case TOGGLEPAUSE:
				is_paused = ~is_paused;
				break;
			case RESTART:
				reset_grid(gw, gh, grid);
				random_init(gw, gh, grid);
				is_paused = 0;
				break;
			case NEXT:
				if (is_paused) {
					next_gen(gw, gh, grid);
					display_status(wstartx,STATUS_Y,gen);
					display(cwin, gw, gh, grid);
					wclear(cwin);
					gen++;
				}
				break;
		}

		if (is_paused == 0) {
			display_status(wstartx,STATUS_Y,gen);
			display(cwin, gw, gh, grid );
			next_gen(gw, gh, grid);
			wclear(cwin);
			gen++;
			usleep(100*1000);
		}
	}
	endwin();
	return 0;
}
