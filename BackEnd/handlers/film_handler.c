#include "handlers/film_handler.h"
#include "DB/film_db.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

void handle_get_all_films(SOCKET client_socket) {
    char *json_str = get_all_films_json();

    if (!json_str) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 27\r\n\r\n"
            "Errore nel recupero dei film";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    // Crea header HTTP separato con lunghezza corretta del JSON
    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n",
        strlen(json_str));

    // Invia prima header, poi body JSON
    send(client_socket, header, header_len, 0);
    send(client_socket, json_str, strlen(json_str), 0);

    free(json_str);
}

void handle_get_popular_films(SOCKET client_socket) {
    char *json_str = get_popular_films_json();

    if (!json_str) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 27\r\n\r\n"
            "Errore nel recupero dei film";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n",
        strlen(json_str));

    send(client_socket, header, header_len, 0);
    send(client_socket, json_str, strlen(json_str), 0);

    free(json_str);
}


