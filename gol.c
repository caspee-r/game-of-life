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

#include <asm-generic/ioctls.h>
#include <ncurses.h>
//#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define CLEAR_CMD "\033[2J\033[H"

#define ALIVE_CELL_CHAR '#'
#define DEAD_CELL_CHAR '.'

#define WIDTH 30
#define HEIGHT 80

#define SPEED 500


typedef enum {
	DEAD,
	ALIVE,
} State;

typedef struct {
	State state;
	// int neighbors;
} Cell;


struct termios originalTermios;

void random_init(const int w, const int h, Cell grid[h][w]) {
	int i, j;
	srand(time(NULL));
	for (i = 1; i < h  ; i = i+2) {
		for (j = 0; j < w; j++) {
			if (rand() % 2 == 0 && rand() % 3 == 0){
				grid[i][j].state = ALIVE;
				continue;
			}
			grid[i][j].state = DEAD;
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
						continue; // cheking the main cell, but we want to check the
								  // neighbors
					int row = (i+k) % h;
					int col = (j+l) % w;
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
	for (int i = 0;i < h;i++){
		for (int j = 0; j < w; j++) {
			grid[i][j] = next_grid[i][j];
		}
	}
}

void display(WINDOW* win,const int width, const  int height, const Cell grid[height][width], int gen){
	//size_t padd = (*row - h)/ 2;
	//wprintw(win,"\n");
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (grid[i][j].state == ALIVE) {
				wprintw(win,"%c", ALIVE_CELL_CHAR);
			} else {
				wprintw(win,"%c", DEAD_CELL_CHAR);
			}
		}
		wprintw(win,"\n");
	}

}



void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}

void get_terminal_size(int* rows, int* cols){
	struct winsize ws;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws);
	*rows = ws.ws_row;
	*cols = ws.ws_col;
}

WINDOW* create_container(int height, int width, int starty, int startx){
	WINDOW* local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);
	wrefresh(local_win);
	return local_win;
}

int main() {
	WINDOW* cwin; 
	int wstartx, wstarty, wwidth, wheight;
	int rows,cols;
	initscr();
	cbreak();
	noecho();
	start_color();
	keypad(stdscr, TRUE);
	wheight =  LINES - 4; 
	wwidth =  COLS / 2 ;
	wstarty = (LINES - wheight)/2;
	wstartx = (COLS - wwidth) / 2;	/* of the window		*/
	int gh = wheight - 1;
	int gw = wwidth - 1;
	int gen = 0;
	Cell grid[gh][gw];
	cwin = newwin(wheight, wwidth, wstarty, wstartx);
	random_init(gw, gh, grid);
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	refresh();
	while (1) {
		attron(A_BOLD | COLOR_PAIR(1));
		mvprintw(1, wstartx, "Generation:%d",gen);
		attroff(A_BOLD | COLOR_PAIR(1));
		refresh();
		display(cwin,gw, gh, grid,gen);
		box(cwin,0,0);
		wrefresh(cwin);
		usleep(SPEED * 1000);
		next_gen(gw, gh, grid);
		wclear(cwin);
		gen++;
		//sleep(1);
	}
	endwin();
	return 0;

}
