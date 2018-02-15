#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <ncurses.h>

#include "utils.h"
#include "player.h"
#include "cJSON.h"

#define CODE_200 "OK"
#define CODE_404 "Not Found"

void error(char *msg) {
	end_player();
	perror(msg);
	exit(1);
}

void setStatusCode(char *buf, int code) {
	char *msg;
	switch(code) {
		case 200:
			msg = CODE_200;
			break;
		case 404:
			msg = CODE_404;
			break;
		default:
			msg = '\0';
	}
	sprintf(buf, "HTTP/1.1 %d %s\r\n", code, msg);
}

void setHeader(char *buf, const char *name, const char *value) {
	char header[128];
	sprintf(header, "%s: %s\r\n", name, value);
	strcat(buf, header);
}

int parseSong(const char *str, char (*buf)[SONGDATASTRING_SIZE]) {
	cJSON *baseJSON = cJSON_Parse(str);
	cJSON *item = cJSON_GetObjectItemCaseSensitive(baseJSON, "item");
	if(!baseJSON || !item || !item->child) { // likely switching between songs
		buf[0][0] = '\0';
		buf[1][0] = '\0';
		return 2;
	}
	cJSON *title = cJSON_GetObjectItemCaseSensitive(item, "name");
	cJSON *artists = cJSON_GetObjectItemCaseSensitive(item, "artists");
	strcpy(buf[0], title->valuestring);
	cJSON *artist = NULL;
	int artistIndex = 0; // index of buf to put artist name into
	cJSON_ArrayForEach(artist, artists) {
		if(artistIndex < SONGDATA_SIZE) {
			cJSON *artistName = cJSON_GetObjectItemCaseSensitive(artist, "name");
			strcpy(buf[++artistIndex], artistName->valuestring);
		} else {
			break;
		}
	}

	cJSON_Delete(baseJSON);

	return ++artistIndex; // equal to the number of strings written
	// TODO: maybe use malloc instead of taking array of strings

}

void parseTimes(const char *str, int *prog, int *dur) {
	cJSON *baseJSON = cJSON_Parse(str);
	cJSON *progress = cJSON_GetObjectItemCaseSensitive(baseJSON, "progress_ms");
	cJSON *item = cJSON_GetObjectItemCaseSensitive(baseJSON, "item");
	if(!baseJSON || !item || !item->child) {
		*prog = 0;
		*dur = 1;
		return;
	}
	cJSON *duration = cJSON_GetObjectItemCaseSensitive(item, "duration_ms");
	*prog = progress->valueint;
	*dur = duration->valueint;
}

void parsePlayState(const char *str, int *playState) {
	cJSON *baseJSON = cJSON_Parse(str);
	if(!baseJSON) {
		*playState = 0;
		return;
	}
	cJSON *playStateJSON = cJSON_GetObjectItemCaseSensitive(baseJSON, "is_playing");
	*playState = playStateJSON->valueint;
}
