#include "head.h"

/**
 * Authentication process.
 */
char authentication(Auth *auth) {
	char *line = NULL;
	char temp[MAX_SZ];
	size_t len = 0;
	FILE *fp = fopen("cred.txt", "r");
	if (fp == NULL) {
		perror("fopen");
		exit(1);
	}
	while (getline(&line, &len, fp) != -1) {
		clean(line);
		sprintf(temp, "%s,%s", auth->username, auth->password);
		if (strcmp(line, temp) == 0) {
			return PASS;
		}
		sprintf(temp, "%s,", auth->username);
		if (strncmp(line, temp, strlen(temp)) == 0) {
			return FAIL_PASS_NO_MATCH;
		}
	}
	return FAIL_NO_USER;
}

/**
 * Start the server.
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
	addr.sin_port = htons(SERV_C_UDP);

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
	Auth auth;
	char response;

	sock = start_server();

	printf("The ServerC is up and running using UDP on port %d.\n", SERV_C_UDP);

	while (1) {
		if (recvfrom(sock, &auth, sizeof(Auth), 0, (struct sockaddr *)&addr, &len) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("The ServerC received an authentication request from the Main Server.\n");
		response = authentication(&auth);
		if (sendto(sock, &response, sizeof(char), 0, (struct sockaddr *)&addr, len) == -1) {
			perror("sendto");
			exit(1);
		}

		printf("The ServerC finished sending the response to the Main Server.\n");
	}

	return 0;
}
