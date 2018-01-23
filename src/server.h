#ifndef SERVER_H
#define SERVER_H

void init_server(char *code);
CURL * init_curl();
void post_spotify(CURL *curl, const char *code, void *buf);

#endif
