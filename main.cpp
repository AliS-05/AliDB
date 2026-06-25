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
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <cctype>
#define MAXBUFSIZE 4096
std::unordered_map<std::string, std::unique_ptr<ValueObject>> storageMap;

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

std::string extractKey(const std::string &userCommand){
	size_t firstCRLF = userCommand.find("\r\n");
	if(firstCRLF == std::string::npos) return std::string("Error");

	size_t secondCRLF = userCommand.find("\r\n", firstCRLF + 2);
	if(secondCRLF == std::string::npos) return std::string("Error");

	std::string key = userCommand.substr(firstCRLF + 2, secondCRLF - (firstCRLF + 2));
	return key;
}

bool checkExists(const std::string &Key){
	auto iter = storageMap.find(Key);
	if (iter == storageMap.end()){
		return false;
	}
	return true;
}


void handleSet(std::string &userCommand){
	//SET\r\nMyKey\r\n"Hello, World!"\r\n
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
		newObj->valueStr = parts[2];
		storageMap[parts[1]] = std::move(newObj);
		std::cout << "Successfully stored key: " << parts[1] << "with value: " << parts[2] << std::endl;
	}
}

auto searchMap(const std::string &Key){
	return storageMap.find(Key);
}

std::optional<std::string> handleGet(std::string &userCommand){
	// GET\r\nALI\r\n
	size_t firstCRLF = userCommand.find("\r\n");
	if(firstCRLF == std::string::npos) return std::nullopt;
	size_t secondCRLF = userCommand.find("\r\n", firstCRLF + 2);

	auto keyToFind = userCommand.substr(firstCRLF + 2, secondCRLF - (firstCRLF + 2)); // KEY

	auto valueOfKey = searchMap(keyToFind);
	if(valueOfKey == storageMap.end()){
		return std::nullopt;
	}
	//dereference iterators to unique_ptr
	return valueOfKey->second->valueStr;
}

Code handleDel(const std::string &userCommand){
	size_t firstCRLF = userCommand.find("\r\n");
	if(firstCRLF == std::string::npos) return Code::ERROR;

	size_t secondCRLF = userCommand.find("\r\n", firstCRLF + 2);
	if(secondCRLF == std::string::npos)return Code::ERROR;

	std::string delObj  = userCommand.substr(firstCRLF + 2, secondCRLF - (firstCRLF + 2)); // KEY
	
	auto it = searchMap(delObj);

	if(it == storageMap.end()){
		return Code::ERROR;
	}
	storageMap.erase(it);
	return Code::SUCCESS;
}

Code handleIncrement(const std::string &userCommand){
	auto key = extractKey(userCommand);
	std::cout << key << std::endl;
	auto val = searchMap(key);
	std::cout << val->second->valueStr << std::endl;
	for(auto d : val->second->valueStr){
		if(!isdigit(static_cast<unsigned char>(d)))
			return Code::ERROR;
	}
	auto res = std::stoi(val->second->valueStr);
	res++;
	std::cout << res << std::endl;
	val->second->valueStr = std::to_string(res);
	return Code::SUCCESS;
}

Code handleDecrement(const std::string &userCommand){
	auto key = extractKey(userCommand);
	auto val = searchMap(key);
	for(auto d : val->second->valueStr){
		if(!isdigit(static_cast<unsigned char>(d)))
			return Code::ERROR;
	}
	auto res = std::stoi(val->second->valueStr);
	res--;
	val->second->valueStr = std::to_string(res);
	return Code::SUCCESS;
}

Method parseMethod(const std::string &userCommand){
	auto firstCRLF = userCommand.find("\r\n");
	auto command = userCommand.substr(0, firstCRLF);
	for (auto& c : command)
		c = std::toupper(static_cast<unsigned char>(c));

	if (command == "PING") return Method::M_PING;
	if (command == "SET") return Method::M_SET;
	if (command == "GET") return Method::M_GET;
	if (command == "DEL") return Method::M_DEL;
	if (command == "CHECK") return Method::M_CHECK;
	if (command == "INCR") return Method::M_INCR;
	if (command == "DECR") return Method::M_DECR;
	return Method::M_UNKNOWN;
} 	



//std::unique_ptr<ValueObject>
void parseUserCommand(int client_fd, std::string &userCommand){ //checks for mehthod user requested
	Method commandMethod = parseMethod(userCommand);
	switch(commandMethod){
		case Method::M_PING:
			write(client_fd, "PONG\r\n", 6);
			break;
		case Method::M_SET:
			write(client_fd, "SET Received\r\n", 13);
			handleSet(userCommand);
			break;
		case Method::M_GET: {
			auto val = handleGet(userCommand);
			if(!val){
				write(client_fd, "KEY NOT FOUND\r\n", 14);
			}else{
				write(client_fd, val->data(), val->size());
				write(client_fd, "\r\n", 2);
			}
			break;
		}
		case Method::M_DEL:
			std::cout << "Looking for deleted key.." << std::endl;
			if(handleDel(userCommand) == Code::SUCCESS){
				write(client_fd, "KEY DELETED\r\n", 13);
			} else{
				write(client_fd, "KEY NOT FOUND\r\n", 14);
			}
			break;
		case Method::M_CHECK:
			if(checkExists(extractKey(userCommand))){
					write(client_fd, "TRUE\r\n", strlen("TRUE\r\n"));
			} else {
				write(client_fd, "FALSE\r\n", strlen("FALSE\r\n"));
			}
			break;
		case Method::M_INCR:
			if(handleIncrement(userCommand) == Code::SUCCESS){
				write(client_fd, "Value Incremented", strlen("Value Incremented"));
			} else {
				write(client_fd, "Error Incrementing", strlen("Error Incrementing"));
			}
			break;
		case Method::M_DECR:
			if(handleDecrement(userCommand) == Code::SUCCESS){
				write(client_fd, "Value Decremented", strlen("Value Decrementing"));
			} else {
				write(client_fd, "Error Decrementing", strlen("Error Decrementing"));
			}
			break;
		default:
			std::cout << "Text Received: " + userCommand << std::endl;
			write(client_fd, "Not sure", strlen("Not sure"));
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

