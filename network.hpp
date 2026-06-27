#pragma once

int create_socket();
void bind_and_listen(int fd);
int accept_client(int fd);
void handle_client(int client_fd, std::string& buffer);
