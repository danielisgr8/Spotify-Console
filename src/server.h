#ifndef SERVER_H
#define SERVER_H

void init_server(char *code);
CURL * init_curl();
void post_spotify(CURL *curl, const char *code, char *buf);
void getSongData(CURL *curl, const char *token, char *buf);
int changePlayState(CURL *curl, const char *token, int state);

#endif
