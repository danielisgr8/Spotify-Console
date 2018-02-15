#ifndef UTILS_H
#define UTILS_H

#define SONGDATA_SIZE 10
#define SONGDATASTRING_SIZE 128

void error(char *msg);
void setStatusCode(char *buf, int code);
void setHeader(char *buf, const char *name, const char *value);
int parseSong(const char *str, char (*buf)[SONGDATASTRING_SIZE]);
void parseTimes(const char *str, int *prog, int *dur);
void parsePlayState(const char *str, int *playState);

#endif
