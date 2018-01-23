#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

#define CODE_200 "OK"
#define CODE_404 "Not Found"

void error(char *msg) {
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

void exec(const char *expr, const char *string, int nmatch, char *results[]) {
	regex_t regex;
	int regStatus;
	char errbuf[128];
	regmatch_t matches[nmatch + 1]; // first element will be whole regex match, not a specific match
	int i;

	regStatus = regcomp(&regex, expr, 0);
	if(regStatus) {
		regerror(regStatus, &regex, errbuf, 128);
		error(errbuf);
	}

	regStatus = regexec(&regex, string, nmatch + 1, matches, 0);
	if(regStatus) {
		regerror(regStatus, &regex, errbuf, 128);
		error(errbuf);
	}
	for(i = 1; i < nmatch + 1; i++) {
		strncpy(results[i - 1], string + matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so);
		results[i - 1][matches[i].rm_eo - matches[i].rm_so] = '\0';
	}

	regfree(&regex);
}
