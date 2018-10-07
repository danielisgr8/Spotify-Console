#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <curl/curl.h>
#include <ncurses.h>

#include "server.h"
#include "utils.h"
#include "cJSON.h"

#define BUFSIZE 1024
#define CLIENT_ID "fb87a8dcc6504073a292ae657458c3ea"
#define CLIENT_SECRET "90ee5523614c46bca967600a038e7286"

struct Memory {
	char *buf;
	size_t size;
};

void readline(int socket, char *buffer) {
	char nextChar = '\0';
	int offset = 0;
	int readCode;
	while(nextChar != '\n') {
		if((readCode = read(socket, buffer + offset, 1)) < 0) {
			error("Error in readline() read()");
		} else if(readCode == 0) { // reached eof before newline
			break;
		} else {
			nextChar = buffer[offset++];
		}
	}
	buffer[offset] = '\0';
}

void init_server(char *code) {
	int listenfd,
		port = 80, // must put port in Spotify developer console, so can't allow user-decided port
		clientfd,
		optval = 1;
	socklen_t clientlen;
	struct sockaddr_in serveraddr, clientaddr;

	signal(SIGPIPE, SIG_IGN); // ignores SIGPIPE signal, as it unnecessarily crashes the program

	// create TCP socket
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("Error in socket()");
	}

	// removes wait when restarting server
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
		error("Error in setsockopt()");
	}

	// set up serveraddr
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short) port);

	// bind socket
	if(bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		error("Error in bind()");
	}

	// open socket for listening with a queue size of 10
	if(listen(listenfd, 10) < 0) {
		error("Error in listen()");
	}

	int done = 0;
	clientlen = sizeof(clientaddr);
	while(1) {
		if((clientfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
			error("Error in accept()");
		}

		int filefd = -1;

		char reqline[BUFSIZE]; // stores first http request line
		char path[BUFSIZE]; // stores path requested
		readline(clientfd, reqline);
		sscanf(reqline, "%*s %s", path); // gets path (technically, the second space-delimited word in the request)

		// holds headers string
		char headerbuf[BUFSIZE];

		// open html file that will be the body of the response
		// opened before setting headers so Content-Length is known
		if(strcmp(path, "/") == 0) { // default directory
			filefd = open("index.html", O_RDONLY);
		} else {
			char test[16];
			strncpy(test, path, 15); // length of "/callback?code="
			test[15] = '\0';
			if(strcmp(test, "/callback?code=") == 0) {
				strcpy(code, path + 15); // path + 15 points to code query string value

				filefd = open("done.html", O_RDONLY);

				done = 1;
			}
		}

		off_t offset = 0;
		int newBytes;
		if(filefd != -1) { // file opened successfully
			struct stat filestat;
			fstat(filefd, &filestat);
			int size = filestat.st_size;

			setStatusCode(headerbuf, 200);
			setHeader(headerbuf, "Connection", "close");
			setHeader(headerbuf, "Content-Type", "text/html");
			char sizeStr[5];
			sprintf(sizeStr, "%d", size);
			setHeader(headerbuf, "Content-Length", sizeStr);
			strcat(headerbuf, "\r\n");

			while(offset < strlen(headerbuf)) {
				if((newBytes = write(clientfd, headerbuf + offset, strlen(headerbuf) - offset)) < 0) {
					error("Error in write() (headers)");
				} else {
					offset += newBytes;
				}
			}

			offset = 0;
			while(offset < size) {
				if(sendfile(clientfd, filefd, &offset, size - offset) < 0) {
					break;
				}
			}
		} else {
			setStatusCode(headerbuf, 404);
			setHeader(headerbuf, "Connection", "close");
			strcat(headerbuf, "\r\n");

			while(offset < strlen(headerbuf)) {
				if((newBytes = write(clientfd, headerbuf + offset, strlen(headerbuf) - offset)) < 0) {
					error("Error in write() (headers)");
				} else {
					offset += newBytes;
				}
			}
		}

		close(clientfd);

		if(done) {
			break;
		}
	}
	close(listenfd);
}

CURL * init_curl() {
	curl_global_init(CURL_GLOBAL_ALL);
	return curl_easy_init();
}

size_t spotify_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
	struct Memory *mem = (struct Memory *)userdata;
	size_t bytec = size * nmemb;
	memcpy(mem->buf + mem->size, ptr, bytec);
	mem->size += bytec;
	return bytec;
}

void post_spotify(CURL *curl, const char *code, char *buf) {
	if(curl) {
		CURLcode res;
		char response[512];
		struct Memory resData;
		resData.buf = response;
		resData.size = 0;
		char body[512];

		curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
		sprintf(body, "grant_type=authorization_code&code=%s&redirect_uri=http%%3A%%2F%%2Flocalhost%%2Fcallback", code);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_USERNAME, CLIENT_ID);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, CLIENT_SECRET);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, spotify_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resData);

		res = curl_easy_perform(curl);
		response[resData.size] = '\0';
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			exit(1);
		}

		cJSON *json = cJSON_Parse(response);
		cJSON *tokenJSON = cJSON_GetObjectItemCaseSensitive(json, "access_token");
		strcpy(buf, tokenJSON->valuestring);
	} else {
		error("curl failed initialization");
	}
}

void getSongData(CURL *curl, const char *token, char *buf) {
	CURLcode res;
	struct Memory resData;
	resData.buf = buf;
	resData.size = 0;

	char authHeader[1024];
	sprintf(authHeader, "Authorization: Bearer %s", token);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, authHeader);
	headers = curl_slist_append(headers, "Content-Length: 0");

	curl_easy_reset(curl);
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.spotify.com/v1/me/player/currently-playing");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, spotify_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resData);

	res = curl_easy_perform(curl);
	buf[resData.size] = '\0';
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		exit(1);
	}

	curl_slist_free_all(headers);

	long responseCode;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	if(responseCode != 200) {
		error("Access token expired");
	}
}

int changePlayState(CURL *curl, const char *token, int state) {
	CURLcode res;
	char *url;
	if(state) { // currently playing, change to pause
		url = "https://api.spotify.com/v1/me/player/pause";
	} else { // currently pause, change to play
		url = "https://api.spotify.com/v1/me/player/play";
	}

	char authHeader[1024];
	sprintf(authHeader, "Authorization: Bearer %s", token);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, authHeader);
	headers = curl_slist_append(headers, "Content-Length: 0");

	curl_easy_reset(curl);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1); // don't output response (error response is to be expected)

	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		exit(1);
	}

	curl_slist_free_all(headers);

	long responseCode;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
	if(responseCode != 204) {
		return changePlayState(curl, token, state - 1); // assumed wrong playback state, try again with other state
	}
	return state ? 0 : 1;
}
