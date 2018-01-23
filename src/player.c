#include <ncurses.h>

void init_player() {
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	mvaddstr(0, 0, "Authorize your Spotify account at http://localhost");
	refresh();
}

void end_player() {
	endwin();
}
