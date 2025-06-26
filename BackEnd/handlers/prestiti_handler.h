#ifndef PRESTITI_HANDLER_H
#define PRESTITI_HANDLER_H

#include <sys/socket.h>   // per socket(), send(), recv()
#include <netinet/in.h>   // per sockaddr_in
#include <arpa/inet.h>    // per inet_ntoa(), htons(), ecc.
#include <unistd.h>       // per close()
#include <stdio.h>        // per printf/debug

typedef int SOCKET;  // compatibilità con codice Windows

void handle_post_prestito(SOCKET client_socket, const char *request);
void handle_prestiti_options(SOCKET client_socket, const char *request);
void handle_get_prestiti(SOCKET client_socket, const char *request);
void handle_put_prestito(SOCKET client_socket, const char *request);

#endif
