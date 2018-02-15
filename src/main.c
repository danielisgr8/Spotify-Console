#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"
#include "player.h"
#include "utils.h"

int main(int argc, char *argv[]) {
	CURL *curl;
	char code[512];
	char access_token[512];

	curl = init_curl();
	init_player(); // initialize ncurses 
	display_init(); // prompt user to authorize Spotify account
	init_server(code); // intialize http server for Spotify authorization
	post_spotify(curl, code, access_token); // sends POST request to Spotify to recieve an access token for the user
	display_player(curl, access_token); // begin loop of getting playback data and receiving user input

	end_player(); // deinitialize ncurses
	curl_easy_cleanup(curl); // cleanup curl, it won't be used again
	curl_global_cleanup();

	return 0;
}

// TODO: using refresh token
