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

std::vector<std::unique_ptr<ValueObject>> storageVector;

auto searchVector(std::string &Key){
	for(const auto& obj : storageVector){
		if(obj->keyStr == Key){
			return obj->valueStr;
		}
	}
	return std::string{};
}

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

void handleSet(std::string &userCommand){
	//SET\r\nmykey\r\n"Hello, Redis!"\r\n
	auto newObj = std::make_unique<ValueObject>(ValueType::STRING);

	std::vector<std::string> parts;
	size_t start = 0;
	size_t end = userCommand.find("\r\n");

	// Split the command into parts based on \r\n
	while (end != std::string::npos) {
		parts.push_back(userCommand.substr(start, end - start));
		start = end + 2; // Skip the \r\n
		end = userCommand.find("\r\n", start);
	}

	// Now parts[0] is "SET"
	// parts[1] is the Key
	// parts[2] is the Value
	if (parts.size() >= 3) {
		newObj->keyStr = parts[1];
		newObj->valueStr = parts[2];
		storageVector.push_back(std::move(newObj)); // adds to the end of vector
		std::cout << "Successfully stored key: " << storageVector.back()->keyStr << std::endl;
	}
}

std::string handleGet(std::string &userCommand){
	// GET\r\nALI\r\n
	size_t firstCRLF = userCommand.find("\r\n");
	if(firstCRLF == std::string::npos) return userCommand;
	size_t secondCRLF = userCommand.find("\r\n", firstCRLF + 2);

	auto keyToFind = userCommand.substr(firstCRLF + 2, secondCRLF - (firstCRLF + 2)); // KEY
	// DEBUG: See the key with literal characters
	std::cout << "Searching for key: [";
	for(char c : keyToFind) {
	if(c == '\r') std::cout << "\\r";
	else if(c == '\n') std::cout << "\\n";
	else std::cout << c;
	}
	std::cout << "]" << std::endl;
	auto valueOfKey = searchVector(keyToFind);
	std::cout << valueOfKey << std::endl;
	return valueOfKey;
}

Method parseMethod(const std::string &userCommand){
	auto firstCRLF = userCommand.find("\r\n");
	auto command = userCommand.substr(0, firstCRLF);
	if (command == "PING") return Method::M_PING;
	if (command == "SET") return Method::M_SET;
	if (command == "GET") return Method::M_GET;
	if (command == "DEL") return Method::M_DEL;
	return Method::M_UNKNOWN;
} 	

//std::unique_ptr<ValueObject>
void parseUserCommand(int client_fd, std::string &userCommand){ //checks for mehthod user requested
	Method commandMethod = parseMethod(userCommand);
	// key and value getter function ?
	switch(commandMethod){
		case Method::M_PING:
			write(client_fd, "PONG\r\n", 6);
			break;
		case Method::M_SET:
			write(client_fd, "SET Received\r\n", 13);
			handleSet(userCommand);
			break;
		case Method::M_GET: {
			auto key = handleGet(userCommand);
			if(key.empty()){
				write(client_fd, "KEY NOT FOUND\r\n", 14);
			}else{
				write(client_fd, key.data(), key.size());
				write(client_fd, "\r\n", 2);
			}
			break;
		}

		case Method::M_DEL:
			//TODO
			break;
		default:
			std::cout << "Text Received: " + userCommand << std::endl;
			write(client_fd, "Not sure", 4);
			break;
	}
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
		//std::unique_ptr<ValueObject> userObject = 
		parseUserCommand(client_fd, userCommand);
	}
	close(client_fd);
	close(sock_fd);
	return 0;
}

