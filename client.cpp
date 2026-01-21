#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

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

    std::cout << "Connected. Type message and press Enter:\n";

    char buf[1024];

    // read one line from user
    if (!std::cin.getline(buf, sizeof(buf))) {
        std::cerr << "stdin error\n";
        return 1;
    }

    send(sock, buf, strlen(buf), 0);

    // receive response
    int n = recv(sock, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Received:\n" << buf << std::endl;
    }

    close(sock);
    return 0;
}

