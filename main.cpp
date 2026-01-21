#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
int main(){
	char buffer[1025];
	int sockfd, bind_code, listen_code;
	struct sockaddr_in serv_addr, client_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0){
		std::perror("Error opening socket");
		std::exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //IPv4 address family
	serv_addr.sin_port = htons(8080); //port 8080
	serv_addr.sin_addr.s_addr = INADDR_ANY; //binds to all available ip's
	
	if((bind_code = bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) != 0){
			std::perror("Error binding socket");
			std::exit(EXIT_FAILURE);
	}

	if((listen_code = listen(sockfd, 1024)) != 0){
		std::perror("Error listening");
		std::exit(EXIT_FAILURE);
	}

	socklen_t client_size = sizeof(client_addr); 
	int client_fd = accept(sockfd, (struct sockaddr*) &client_addr, &client_size);
	if(client_fd < 0){
		std::perror("Error accepting");
		std::exit(EXIT_FAILURE);
	}
	int n;

	while((n = read(client_fd, buffer, sizeof(buffer)-1)) > 0){
		buffer[n] = '\0';
		std::cout << buffer << std::endl;
	}
	
	close(sockfd);
	close(client_fd);
	return 0;
}
