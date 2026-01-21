#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <string>
#define MAXBUFSIZE 4096

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
	while ((n = read(client_fd, tmp, sizeof(tmp))) > 0) {
		buffer.append(tmp, n);
		std::cout << buffer << std::endl;
	}
	return;
}

void parse_command(int sock_fd, std::string &command){
	
	if(command.find("PING") != std::string::npos){ //actually if this is located at position 0 evaluates to false
		write(sock_fd, "PONG\r\n", 7);
	}
}

int main() {
	std::string userCommand;
	int sock_fd = create_socket();
	bind_and_listen(sock_fd);

	int client_fd = accept_client(sock_fd);
	handle_client(client_fd, userCommand);


	//networking part done. need to parse command from user
	parse_command(sock_fd, userCommand);

	close(client_fd);
	close(sock_fd);
	return 0;
}

