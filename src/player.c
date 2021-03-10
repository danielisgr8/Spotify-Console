#include <ncurses.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>

#include "player.h"
#include "server.h"
#include "utils.h"

#define COORDY_PLAY_STATE   dimY - 4
#define COORDX_PLAY_STATE   dimX / 2
#define COORDY_TIME         dimY - 2
#define COORDY_TITLE_ARTIST 2

void print_top_bottom(int y);
void print_middle(int y);
void printTime(int y, int timeChars);
void printPlayState(int y, int x, int val);
void updatePlayer(CURL *curl, const char *access_token);
void updateSong(char *strc);
void updateTime(char *strc);
void updatePlayState(char *str);
void update_player(int prop, int val);
void update_player_string(int prop, const char *val);
void player_toggle();
void clearLine(int line);

int dimY;
int dimX;
int lastPlayState;

void init_player() {
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
}

void display_init() {
	mvaddstr(0, 0, "Authorize your Spotify account at http://localhost");
	refresh();
}

void display_player(CURL *curl, const char *access_token) {
	int i;

	getmaxyx(stdscr, dimY, dimX);

	print_top_bottom(0);
	for(i = 1; i < dimY - 1; i++) {
		print_middle(i);
	}
	print_top_bottom(dimY - 1);
	refresh();

	int ch, cycles = 0;
	while(1) {
		ch = getch();
		if(ch != ERR) {
			if(ch == ' ') {
				lastPlayState = changePlayState(curl, access_token, lastPlayState);
				update_player(PROP_PLAY_STATE, lastPlayState);
			} else if(ch == 'q') {
				break;
			}
		}
		if(cycles % 10 == 0) {
			updatePlayer(curl, access_token);
			cycles = 0;
		}
		usleep(100000);
		cycles += 1;
	}
}

void print_top_bottom(int y) {
	int i;
	mvaddch(y, 0, '|');
	for(i = 1; i < dimX - 1; i++) {
		addch('-');
	}
	addch('|');
}

void print_middle(int y) {
	int i;
	mvaddch(y, 0, '|');
	for(i = 1; i < dimX - 1; i++) {
		addch(' ');
	}
	addch('|');
}

void printTime(int y, int timeChars) {
	int i;
	mvaddch(y, 1, '|');
	for(i = 2; i < timeChars + 2; i++) {
		addch('#');
	}
	for(; i < dimX - 2; i++) {
		addch('-');
	}
	addch('|');
}

void printPlayState(int y, int x, int val) {
	mvaddch(y, x, '|');
	if(val == VAL_PLAY) {
		addch('>');
	} else if(val == VAL_PAUSE) {
		addch('|');
	}
}

void updatePlayer(CURL *curl, const char *access_token) {
	char buf[8192];
	getSongData(curl, access_token, buf);
	updateSong(buf);
	updateTime(buf);
	updatePlayState(buf);
}

void updateSong(char *str) {
	if(str[0] == '\0') {
		update_player_string(PROP_TITLE_ARTIST, "No song playing");
		return;
	}

	char songData[SONGDATA_SIZE][SONGDATASTRING_SIZE];
	char songString[256];
	int written, offset;

	written = parseSong(str, songData);

	offset = sprintf(songString, "%s -", songData[0]);
	int i;
	for(i = 1; i < written - 1; i++) {
		offset += sprintf(songString + offset, " %s,", songData[i]);
	}
	offset += sprintf(songString + offset, " %s", songData[i]);
	songString[offset] = '\0';

	update_player_string(PROP_TITLE_ARTIST, songString);
}

void updateTime(char *str) {
	int prog, dur;
	if(str[0] == '\0') {
		prog = 0;
		dur = 1;
	} else {
		parseTimes(str, &prog, &dur);
	}
	float percent = (float) prog / dur;
	float numChars = percent * (dimX - 4);
	update_player(PROP_TIME, (int) numChars);
}

void updatePlayState(char *str) {
	parsePlayState(str, &lastPlayState);
	update_player(PROP_PLAY_STATE, lastPlayState);
}

void update_player(int prop, int val) {
	switch(prop) {
		case PROP_PLAY_STATE:
			printPlayState(COORDY_PLAY_STATE, COORDX_PLAY_STATE, val);
			break;
		case PROP_TIME:
			printTime(COORDY_TIME, val);
	}
	refresh();
}

void update_player_string(int prop, const char *val) {
	int offset = (dimX - strlen(val)) / 2; // offset for placing title/artist info
	offset = offset < 1 ? 1 : offset;
	switch(prop) {
		case PROP_TITLE_ARTIST:
			move(COORDY_TITLE_ARTIST, 0);
			clrtoeol();
			mvaddch(COORDY_TITLE_ARTIST, 0, '|');
			mvaddch(COORDY_TITLE_ARTIST, dimX - 1, '|');
			mvaddnstr(COORDY_TITLE_ARTIST, offset, val, dimX - 2);
			break;
	}
	refresh();
}

void end_player() {
	endwin();
}

