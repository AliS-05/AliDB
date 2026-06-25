#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return 1;
	}

	sockaddr_in server{};
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
		perror("connect");
		return 1;
	}

	while (true) {
		std::string full, command, key, value;
		std::cout << "Enter command with syntax COMMAND Key [Value]\n";
		std::getline(std::cin, full);
		
		std::stringstream ss(full);

		if(!std::getline(ss, command, ' ')) continue;
		std::getline(ss, key, ' ');
		std::getline(ss, value);

		std::cout << "command = " << command << '\n';
		std::cout << "key     = " << key << '\n';
		std::cout << "value   = " << value << '\n';
		
		command.append("\r\n");
		if(key.size() >0){
			command.append(key);
			command.append("\r\n");
		}

		if(value.size() > 0){
			command.append(value);
			command.append("\r\n");
		}

		send(sock, command.c_str(), command.size(), 0);
		char buf[1024];
		int n = recv(sock, buf, sizeof(buf) - 1, 0);
		if (n > 0) {
			buf[n] = '\0';
			std::cout << "Server response: " << buf << "\n";
		} else {
		    std::cout << "No response or connection closed.\n";
		    break;
		}
	}
	close(sock);
	return 0;
}
