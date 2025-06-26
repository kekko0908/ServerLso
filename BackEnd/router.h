#pragma once

#include <sys/socket.h>   // socket(), send(), recv()
#include <netinet/in.h>   // sockaddr_in
#include <arpa/inet.h>    // inet_addr(), htons(), etc.
#include <unistd.h>       // close()
#include <stdio.h>        // printf, debug

typedef int SOCKET;       // compatibilità con il codice esistente

void route_request(SOCKET client_socket, const char *request);
