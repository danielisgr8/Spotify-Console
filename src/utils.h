#ifndef UTILS_H
#define UTILS_H

void error(char *msg);
void setStatusCode(char *buf, int code);
void setHeader(char *buf, const char *name, const char *value);
void exec(const char *expr, const char *string, int nmatch, char *results[]);

#endif
