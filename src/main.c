#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>

#include "server.h"
#include "player.h"
#include "utils.h"

int main(int argc, char *argv[]) {
	CURL *curl;
	char code[512];
	char access_token[512];

	curl = init_curl();
	// init_player(); // initialize ncurses and tell user to authorize Spotify account
	init_server(code); // intialize http server for Spotify authorization
	post_spotify(curl, code, access_token); // sends POST request to Spotify to recieve an access token for the user
	curl_easy_cleanup(curl); // cleanup curl, it won't be used again
	// end_player(); // deinitialize ncurses
	printf("%s\n", access_token);
	curl_global_cleanup();

	return 0;
}
