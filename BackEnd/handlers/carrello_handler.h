#ifndef CARRELLO_HANDLER_H
#define CARRELLO_HANDLER_H

#include <sys/socket.h>   // socket(), send(), recv()
#include <netinet/in.h>   // struct sockaddr_in
#include <arpa/inet.h>    // inet_ntoa(), htons(), ecc.
#include <unistd.h>       // close()
#include <stdio.h>        // debug, printf

typedef int SOCKET;  // compatibilità con codice WinSock

void handle_post_cart_item(SOCKET client_socket, const char *request);
void handle_carrello_options(SOCKET client_socket);
void handle_get_cart_items(SOCKET client_socket, const char *request);
void handle_delete_cart_item(SOCKET client_socket, const char *request);

#endif
