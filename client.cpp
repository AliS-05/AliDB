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

    while (true) {
        // --- print menu ---
        std::cout << "\nSelect a command:\n";
        std::cout << "1) PING\n";
        std::cout << "2) SET\n";
        std::cout << "3) Send custom text\n";
        std::cout << "4) Quit\n";
        std::cout << "Enter choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(); // discard leftover newline

        std::string command;

        if (choice == 1) {
            command = "PING\r\n";
        } else if (choice == 2) {
            std::string key, value;
            std::cout << "Enter key: ";
            std::getline(std::cin, key);
            std::cout << "Enter value: ";
            std::getline(std::cin, value);
            command = "SET\r\n" + key + " " + value;
        } else if (choice == 3) {
            std::cout << "Enter text to send: ";
            std::getline(std::cin, command);
        } else if (choice == 4) {
            std::cout << "Exiting...\n";
	    send(sock, "EXIT\r\n", 6, 0);
            break;
        } else {
            std::cout << "Invalid choice.\n";
            continue;
        }

        // --- send command ---
        send(sock, command.c_str(), command.size(), 0);

        // --- receive response ---
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

