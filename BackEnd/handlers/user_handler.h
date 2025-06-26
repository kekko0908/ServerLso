#pragma once

#include <sys/socket.h>   // socket functions
#include <netinet/in.h>   // struct sockaddr_in
#include <arpa/inet.h>    // inet_ntoa, htons, etc.
#include <unistd.h>       // close()
#include <stdio.h>        // printf, etc.

// In Linux, i socket sono semplici int
typedef int SOCKET;

void handle_register(SOCKET client_socket, const char *request);
void handle_register_options(SOCKET client_socket);
void handle_login(SOCKET client_socket, const char *body);
void handle_login_options(SOCKET client_socket);         
void handle_get_all_users(SOCKET client_socket);
void handle_user_request(SOCKET client_socket, const char *method, const char *path, const char *body);
void handle_logout(SOCKET client_socket, const char *request);
void handle_logout_options(SOCKET client_socket);
