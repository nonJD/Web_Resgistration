#include "head.h"

/**
 * Read authentication information from console.
 */
void read_auth(Auth *auth) {
	printf("Please enter the username: ");
	fgets(auth->username, sizeof(char) * MAX_SZ, stdin);
	clean(auth->username);
	printf("Please enter the password: ");
	fgets(auth->password, sizeof(char) * MAX_SZ, stdin);
	clean(auth->password);
}

/**
 * Connect to main server.
 */
int connect_to_server_m() {
	struct sockaddr_in addr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERV_M_TCP);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("connect");
		exit(1);
	}
	return sock;
}

/**
 * Get local port number.
 */
int get_local_port(int sock) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	if (getsockname(sock, (struct sockaddr *) &addr, &len) == -1) {
		perror("getsockname");
		exit(1);
	}
	return (int) addr.sin_port;
}

/**
 * Authentication process.
 */
void authentication(int sock, Auth *auth) {
	char response;
	int attempts = 3;
	int port = get_local_port(sock);
	while (attempts > 0) {
		read_auth(auth);

		if (send(sock, (void*) auth, sizeof(Auth), 0) == -1) {
			perror("send");
			exit(1);
		}
		printf("%s sent an authentication request to the main server.\n", auth->username);

		if (recv(sock, &response, sizeof(char), 0) == -1)
		{
			perror("recv");
			exit(1);
		}
		printf("%s received the result of authentication using TCP over port %d. ", auth->username, port);

		if (response == FAIL_NO_USER) {
			printf("Authentication failed: Username Does not exist\n");
		} else if (response == FAIL_PASS_NO_MATCH) {
			printf("Authentication failed: Password does not match\n");
		} else if (response == PASS) {
			printf("Authentication is successful\n");
			break;
		}
		attempts--;
		printf("Attempts remaining: %d\n", attempts);
	}
	if (attempts == 0) {
		printf("Authentication Failed for 3 attempts. Client will shut down.\n");
		exit(0);
	}
}

/**
 * Check if this query is extra.
 */
int is_extra_query(Query *query) {
	int i = 0;
	while (query->code[i] != '\0') {
		if (query->code[i] == ' ') {
			return 1;
		}
		i++;
	}
	return 0;
}

/**
 * Query course process.
 */
void query_course(int sock, Auth *auth) {
	Query query;
	char input[MAX_SZ], response[MAX_SZ];
	memset(input, 0, sizeof(input));
	memset(response, 0, sizeof(response));
	printf("Please enter the course code to query: ");
	fgets(query.code, sizeof(char) * MAX_SZ, stdin);
	clean(query.code);
	if (is_extra_query(&query)) {
		query.category = EXTRA;
	} else {
		printf("Please enter the category (Credit / Professor / Days / CourseName): ");
		fgets(input, sizeof(char) * MAX_SZ, stdin);
		clean(input);
		if (strncmp(input, "Credit", strlen("Credit")) == 0) {
			query.category = CREDIT;
		} else if (strncmp(input, "Professor", strlen("Professor")) == 0) {
			query.category = PROFESSOR;
		} else if (strncmp(input, "Days", strlen("Days")) == 0) {
			query.category = DAYS;
		} else if (strncmp(input, "CourseName", strlen("CourseName")) == 0) {
			query.category = COURSE_NAME;
		} else {
			printf("Bad category: %s\n", input);
			return;
		}
	}

	if (send(sock, (void*) &query, sizeof(Query), 0) == -1) {
		perror("send");
		exit(1);
	}
	printf("%s sent a request to the main server.\n", auth->username);

	memset(response, 0, sizeof(response));
	if (recv(sock, &response, sizeof(response), 0) == -1) {
		perror("recv");
		exit(1);
	}
	printf("The client received the response from the Main server using TCP over port %d.\n", get_local_port(sock));
	printf("%s", response);
}

int main() {
	int sock;
	Auth auth;

	printf("The client is up and running.\n");

	sock = connect_to_server_m();
	authentication(sock, &auth);

	while (1) {
		query_course(sock, &auth);
	}
	return 0;
}
