#include "dataStructures.hpp"
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <memory>
#define MAXBUFSIZE 4096

std::vector<ValueObject> storageVector;

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

std::unique_ptr<ValueObject> handleSet(std::string &userCommand){
	//SET\r\nmykey\r\n"Hello, Redis!"\r\n
	auto newObj = std::make_unique<ValueObject>(ValueType::STRING);

	size_t firstCRLF = userCommand.find("\r\n"); //first \r\n
	if(firstCRLF == std::string::npos) return nullptr;

	size_t secondCRLF = userCommand.find("\r\n", firstCRLF); 
	//skips SET\r\n, then from \n to 
	newObj->keyStr = userCommand.substr(firstCRLF + 2, secondCRLF - (firstCRLF + 2));
	
	size_t thirdCRLF = userCommand.find("\r\n", secondCRLF + 2);
	if(thirdCRLF == std::string::npos){
		newObj->valueStr = userCommand.substr(secondCRLF + 2);
	} else{
		newObj->valueStr = userCommand.substr(secondCRLF + 2, thirdCRLF - (secondCRLF + 2));
	}

	return newObj;
}

std::unique_ptr<ValueObject> parse_userCommand(int client_fd, std::string &userCommand){
	
	if((userCommand.substr(0,6)) == "PING\r\n"){ 
		write(client_fd, "PONG\r\n", 6);
	}if((userCommand.substr(0,5)) == "SET\r\n"){
		//handleSet(userCommand);
		write(client_fd, "SET Received", 13);
		return handleSet(userCommand);
	}
	else{
		std::cout << "Text Received: " + userCommand << std::endl;
		write(client_fd, "DONG", 4);
		return nullptr;
	}
	return nullptr;
}

int main() {
	std::string userCommand;
	int sock_fd = create_socket();
	bind_and_listen(sock_fd);
	int client_fd = accept_client(sock_fd);
	while(userCommand != "EXIT\r\n"){
		userCommand.clear();
		handle_client(client_fd, userCommand);
		//networking part done. need to parse command from user
		std::unique_ptr<ValueObject> userObject = parse_userCommand(client_fd, userCommand);
		std::cout << userObject->keyStr << std::endl;
	}
	close(client_fd);
	close(sock_fd);
	return 0;
}

