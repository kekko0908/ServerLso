#ifndef FILM_HANDLER_H
#define FILM_HANDLER_H

#include <sys/socket.h>   // socket functions
#include <netinet/in.h>   // struct sockaddr_in
#include <arpa/inet.h>    // inet_ntoa, htons, etc.
#include <unistd.h>       // close()
#include <stdio.h>        // printf, debug output

typedef int SOCKET;  // compatibilità con il codice esistente

void handle_get_all_films(SOCKET client_socket);
void handle_get_popular_films(SOCKET client_socket);

#endif
