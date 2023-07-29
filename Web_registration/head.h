#ifndef HEAD
#define HEAD

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAX_SZ 2048

#define PASS 0
#define FAIL_NO_USER 1
#define FAIL_PASS_NO_MATCH 2

#define CREDIT 0
#define PROFESSOR 1
#define DAYS 2
#define COURSE_NAME 3
#define EXTRA 4

#define SERV_C_UDP  21015
#define SERV_CS_UDP 22015
#define SERV_EE_UDP 23015
#define SERV_M_UDP  24015
#define SERV_M_TCP  25015

typedef struct {
	char username[MAX_SZ];
	char password[MAX_SZ];
} Auth;

typedef struct {
	int category;
    char code[MAX_SZ];
} Query;

char* CATEGORY[4] = {"Credit", "Professor", "Days", "CourseName"};

void clean(char *str) {
	int i = 0;
	while (str[i] != '\0') {
		if (str[i] == '\r' || str[i] == '\n') {
			str[i] = '\0';
			return;
		}
		i++;
	}
}

#endif