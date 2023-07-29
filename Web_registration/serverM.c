#include "head.h"

/**
 * Start server.
 */
int start_server() {
	struct sockaddr_in addr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERV_M_TCP);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock, MAX_SZ) == -1) {
		perror("listen");
		exit(1);
	}
	return sock;
}

/**
 * Encryption process.
 */
void encrypt(Auth *auth) {
	int i = 0;
	int shift = 4;
	while (auth->username[i] != '\0') {
		if (auth->username[i] >= '0' && auth->username[i] <= '9')
		{
			auth->username[i] = (auth->username[i] - '0' + shift) % 10 + '0';
		}
		else if (auth->username[i] >= 'A' && auth->username[i] <= 'Z')
		{
			auth->username[i] = (auth->username[i] - 'A' + shift) % 26 + 'A';
		}
		else if (auth->username[i] >= 'a' && auth->username[i] <= 'z')
		{
			auth->username[i] = (auth->username[i] - 'a' + shift) % 26 + 'a';
		}
		i++;
	}
	i = 0;
	while (auth->password[i] != '\0') {
		if (auth->password[i] >= '0' && auth->password[i] <= '9')
		{
			auth->password[i] = (auth->password[i] - '0' + shift) % 10 + '0';
		}
		else if (auth->password[i] >= 'A' && auth->password[i] <= 'Z')
		{
			auth->password[i] = (auth->password[i] - 'A' + shift) % 26 + 'A';
		}
		else if (auth->password[i] >= 'a' && auth->password[i] <= 'z')
		{
			auth->password[i] = (auth->password[i] - 'a' + shift) % 26 + 'a';
		}
		i++;
	}
}

/**
 * Authentication process.
 */
char authentication(Auth *auth) {
	int sock;
	struct sockaddr_in addr;
	socklen_t len;
	char response;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERV_C_UDP);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	len = sizeof(addr);

	if (sendto(sock, (void*) auth, sizeof(Auth), 0, (struct sockaddr *)&addr, len) == -1) {
		perror("sendto");
		exit(1);
	}

	printf("The main server sent an authentication request to serverC.\n");

	if (recvfrom(sock, &response, sizeof(char), 0, (struct sockaddr *) &addr, &len) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("The main server received the result of the authentication request from ServerC using UDP over port %d.\n", SERV_C_UDP);

	close(sock);
	return response;
}

/**
 * Handle authentication process.
 */
int handle_auth(int sock, char **username) {
	Auth auth;
	char response;
	int attempts = 3;
	while (attempts > 0) {
		if (recv(sock, &auth, sizeof(Auth), 0) == -1) {
			perror("recv");
			exit(1);
		}

		*username = strdup(auth.username);
		printf("The main server received the authentication for %s using TCP over port %d.\n", *username, SERV_M_TCP);
		
		encrypt(&auth);
		response = authentication(&auth);
		if (send(sock, &response, sizeof(char), 0) == -1) {
			perror("send");
			exit(1);
		}
		printf("The main server sent the authentication result to the client.\n");
		if (response == PASS) {
			break;
		}
		attempts--;
	}
	return attempts;
}

/**
 * Query course process.
 */
void query_course(Query *query, char response[MAX_SZ]) {
	int sock;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	addr.sin_family = AF_INET;
	if (strncmp(query->code, "EE", strlen("EE")) == 0) {
		addr.sin_port = htons(SERV_EE_UDP);
	} else if (strncmp(query->code, "CS", strlen("CS")) == 0) {
		addr.sin_port = htons(SERV_CS_UDP);
	} else {
		sprintf(response, "Didn't find the course: %s\n", query->code);
		return;
	}

	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	len = sizeof(addr);

	if (sendto(sock, query, sizeof(Query), 0, (struct sockaddr *) &addr, len) == -1) {
		perror("sendto");
		exit(1);
	}

	if (strncmp(query->code, "EE", strlen("EE")) == 0) {
		printf("The main server sent a request to serverEE.\n");
	} else {
		printf("The main server sent a request to serverCS.\n");
	}

	if (recvfrom(sock, response, sizeof(char) * MAX_SZ, 0, (struct sockaddr *)&addr, &len) == -1) {
		perror("recvfrom");
		exit(1);
	}

	if (strncmp(query->code, "EE", strlen("EE")) == 0) {
		printf("The main server received the response from serverEE using UDP over port %d.\n", SERV_M_UDP);
	} else {
		printf("The main server received the response from serverCS using UDP over port %d.\n", SERV_M_UDP);
	}
}

/**
 * Handle query process.
 */
void handle_query(int sock, char *username) {
	Query query;
	char response[MAX_SZ], temp[MAX_SZ], *p;
	while (1) {
		memset(response, 0, sizeof(response));
		memset(temp, 0, sizeof(temp));
		if (recv(sock, &query, sizeof(Query), 0) == -1) {
			perror("recv");
			exit(1);
		}
		if (query.category != EXTRA) {
			query_course(&query, response);
			printf("The main server received from %s to query course %s about %s using TCP over port %d.\n", username, query.code, CATEGORY[query.category], SERV_M_TCP);
			if (strncmp(response, "Didn't find the course:", strlen("Didn't find the course:")) != 0) {
				sprintf(temp, "The %s of %s is %s.\n", CATEGORY[query.category], query.code, response);
			} else {
				sprintf(temp, "%s\n", response);
			}
		} else {
			p = strtok(query.code, " ");
			sprintf(temp, "%s\n", "CourseCode: Credits, Professor, Days, Course Name");
			while (p != NULL) {
				Query sub_query = query;
				sprintf(sub_query.code, "%s", p);
				query_course(&sub_query, response);
				printf("The main server received from %s to query course %s using TCP over port %d.\n", username, query.code, SERV_M_TCP);
				sprintf(temp + strlen(temp), "%s\n", response);
				p = strtok(NULL, " ");
			}
		}
		if (send(sock, temp, sizeof(char) * MAX_SZ, 0) == -1) {
			perror("send");
			exit(1);
		}
		printf("The main server sent the query information to the client.\n");
	}
}

int main() {
	int sock, cli_sock, attempts;
	struct sockaddr_in addr;
	socklen_t len;
	char *username;

	sock = start_server();
	printf("The main server is up and running.\n");

	while (1) {
		memset(&addr, 0, sizeof(addr));
		cli_sock = accept(sock, (struct sockaddr *) &addr, &len);
		if (cli_sock == -1) {
			perror("accept");
			exit(1);
		}
		attempts = handle_auth(cli_sock, &username);
		if (attempts > 0) {
			handle_query(cli_sock, username);
		}
		close(cli_sock);
	}

	return 0;
}
