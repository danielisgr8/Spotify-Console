#ifndef PLAYER_H
#define PLAYER_H

#include <curl/curl.h>

// update_player() properties
#define PROP_PLAY_STATE 0
#define PROP_TIME       1

// update_player_string() properties
#define PROP_TITLE_ARTIST 0

// update_player() values
#define VAL_PLAY  0
#define VAL_PAUSE 1

void init_player();
void display_init();
void display_player(CURL *curl, const char *access_token);
void end_player();

#endif
