#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

int create_socket() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		std::perror("Error opening socket");
		std::exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	return fd;
}

void bind_and_listen(int fd) {
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		std::perror("Error binding socket");
		std::exit(EXIT_FAILURE);
	}

	if (listen(fd, 1024) != 0) {
		std::perror("Error listening");
		std::exit(EXIT_FAILURE);
	}
}

int accept_client(int fd) {
	struct sockaddr_in client_addr;
	socklen_t size = sizeof(client_addr);

	int client_fd = accept(fd, (struct sockaddr*)&client_addr, &size);
	if (client_fd < 0) {
		std::perror("Error accepting");
		std::exit(EXIT_FAILURE);
	}
	return client_fd;
}

void handle_client(int client_fd, std::string& buffer) {
	ssize_t n;
	char tmp[2048];
	n = read(client_fd, tmp, sizeof(tmp));
	buffer.append(tmp, n);
	std::cout << buffer << std::endl;
	return;
}
