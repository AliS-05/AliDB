#include <string>
#include "network.hpp"
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
	auto newObj = std::make_unique<ValueObject>();

	std::vector<std::string> parts;
	size_t start = 0;
	size_t end = userCommand.find("\r\n");

	// Split the command into parts based on \r\n
	//NOTE change while loop ? that doesnt make sense
	while (end != std::string::npos) {
		parts.push_back(userCommand.substr(start, end - start));
		start = end + 2; // Skip the \r\n
		end = userCommand.find("\r\n", start);
	}
	// Now parts[0] is "SET"
	// parts[1] is the Key
	// parts[2] is the Value
	if (parts.size() >= 3) {
		if(auto* condition = std::get_if<std::string>(&newObj->value)){
			*condition = parts[2];
			storageMap[parts[1]] = std::move(newObj);
			std::cout << "Successfully stored key: " << parts[1] << "with value: " << parts[2] << std::endl;
		} else {
			std::cout << "Error with std::variant in handleSet\n" << std::endl;
		}
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
	if(auto* condition = std::get_if<std::string>(&valueOfKey->second->value)){
		return *condition;
	}
	return std::nullopt;
	//return valueOfKey->second->value;
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

	if(auto* condition = std::get_if<std::string>(&val->second->value)){
		std::cout << *condition << std::endl;
		for(auto d : *condition){
			//if any non-digits throw
			if(!isdigit(static_cast<unsigned char>(d)))
				return Code::ERROR;
		}
		auto res = std::stoi(*condition);
		res++;
		std::cout << res << std::endl;
		*condition = std::to_string(res);
		return Code::SUCCESS;
	} else {
		return Code::ERROR;
	}
	return Code::UNKNOWN;
}

Code handleDecrement(const std::string &userCommand){
	auto key = extractKey(userCommand);
	auto val = searchMap(key);
	if(auto* condition = std::get_if<std::string>(&val->second->value)){
		std::cout << *condition << std::endl;
		for(auto d : *condition){
			//if any non-digits throw
			if(!isdigit(static_cast<unsigned char>(d)))
				return Code::ERROR;
		}
		auto res = std::stoi(*condition);
		res--;
		std::cout << res << std::endl;
		*condition = std::to_string(res);
		return Code::SUCCESS;
	} else {
		return Code::ERROR;
	}
	return Code::UNKNOWN;}

Method parseMethod(const std::string &userCommand){
	auto firstCRLF = userCommand.find("\r\n");
	auto command = userCommand.substr(0, firstCRLF);
	for (auto& c : command)
		c = std::toupper(static_cast<unsigned char>(c));

	if (command == "PING")  return Method::PING;
	if (command == "SET")   return Method::SET;
	if (command == "GET")   return Method::GET;
	if (command == "DEL")   return Method::DEL;
	if (command == "CHECK") return Method::CHECK;
	if (command == "INCR")  return Method::INCR;
	if (command == "DECR")  return Method::DECR;
	if (command == "LPUSH") return Method::LPUSH;
	if (command == "RPUSH") return Method::RPUSH;
	return Method::UNKNOWN;
} 	



void parseUserCommand(int client_fd, std::string &userCommand){ //checks for mehthod user requested
	Method commandMethod = parseMethod(userCommand);
	switch(commandMethod){
		case Method::PING:
			write(client_fd, "PONG\r\n", 6);
			break;
		case Method::SET:
			write(client_fd, "SET Received\r\n", 13);
			handleSet(userCommand);
			break;
		case Method::GET: {
			auto val = handleGet(userCommand);
			if(!val){
				write(client_fd, "KEY NOT FOUND\r\n", 14);
			}else{
				write(client_fd, val->data(), val->size());
				write(client_fd, "\r\n", 2);
			}
			break;
		}
		case Method::DEL:
			std::cout << "Looking for deleted key.." << std::endl;
			if(handleDel(userCommand) == Code::SUCCESS){
				write(client_fd, "KEY DELETED\r\n", 13);
			} else{
				write(client_fd, "KEY NOT FOUND\r\n", 14);
			}
			break;
		case Method::CHECK:
			if(checkExists(extractKey(userCommand))){
					write(client_fd, "TRUE\r\n", strlen("TRUE\r\n"));
			} else {
				write(client_fd, "FALSE\r\n", strlen("FALSE\r\n"));
			}
			break;
		case Method::INCR:
			if(handleIncrement(userCommand) == Code::SUCCESS){
				write(client_fd, "Value Incremented", strlen("Value Incremented"));
			} else {
				write(client_fd, "Error Incrementing", strlen("Error Incrementing"));
			}
		break;
		case Method::DECR:
			if(handleDecrement(userCommand) == Code::SUCCESS){
				write(client_fd, "Value Decremented", strlen("Value Decremented"));
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

