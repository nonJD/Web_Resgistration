#include "head.h"

/**
 * Query course process.
 */
void query_course(Query *query, char response[MAX_SZ]) {
	char *line = NULL, *p = NULL;
	char temp[MAX_SZ];
	size_t len = 0;
	FILE *fp = fopen("cs.txt", "r");
	if (fp == NULL) {
		perror("fopen");
		exit(1);
	}
	while (getline(&line, &len, fp) != -1) {
		clean(line);
		sprintf(temp, "%s,", query->code);
		if (strncmp(line, temp, strlen(temp)) == 0) {
			p = strtok(line, ",");
			if (query->category == EXTRA) {
				sprintf(response, "%s: ", p);
				p = strtok(NULL, ",");
				while (p != NULL) {
					sprintf(response + strlen(response), "%s, ", p);
					p = strtok(NULL, ",");
				}
				response[strlen(response) - 2] = '\0';
				printf("The course information has been found: The %s is %s.\n", query->code, response);
			} else {
				p = strtok(NULL, ",");
				if (query->category == CREDIT) {
					sprintf(response, "%s", p);
				}
				p = strtok(NULL, ",");
				if (query->category == PROFESSOR) {
					sprintf(response, "%s", p);
				}
				p = strtok(NULL, ",");
				if (query->category == DAYS) {
					sprintf(response, "%s", p);
				}
				p = strtok(NULL, ",");
				if (query->category == COURSE_NAME) {
					sprintf(response, "%s", p);
				}
				printf("The course information has been found: The %s of %s is %s.\n", CATEGORY[query->category], query->code, response);
			}
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	sprintf(response, "Didn't find the course: %s", query->code);
    printf("Didn't find the course: %s\n", query->code);
}

/**
 * Start server.
 */
int start_server() {
	struct sockaddr_in addr;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERV_CS_UDP);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("bind");
		exit(1);
	}

	return sock;
}

int main() {
	int sock;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	Query query;
	char response[MAX_SZ];

	sock = start_server();

	printf("The ServerCS is up and running using UDP on port %d.\n", SERV_CS_UDP);

	while (1) {
		if (recvfrom(sock, &query, sizeof(Query), 0, (struct sockaddr *)&addr, &len) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("The ServerCS received a request from the Main Server about the %s of %s.\n", CATEGORY[query.category], query.code);
		memset(response, 0, sizeof(response));
		query_course(&query, response);

		if (sendto(sock, &response, sizeof(char) * MAX_SZ, 0, (struct sockaddr *)&addr, len) == -1) {
			perror("sendto");
			exit(1);
		}

		printf("The ServerCS finished sending the response to the Main Server.\n");
	}

	return 0;
}
