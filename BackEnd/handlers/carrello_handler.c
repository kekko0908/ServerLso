#include "handlers/carrello_handler.h"
#include "DB/carrello_db.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cJSON.h"

void handle_post_cart_item(SOCKET client_socket, const char *request) {
    // Estrarre il corpo JSON dalla richiesta HTTP POST
    const char *body = strstr(request, "\r\n\r\n");
    if (!body) {
        // Malformed request
        const char *resp = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }
    body += 4; // skip \r\n\r\n

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        const char *resp = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    cJSON *id_utente_json = cJSON_GetObjectItemCaseSensitive(json, "id_utente");
    cJSON *id_film_json = cJSON_GetObjectItemCaseSensitive(json, "id_film");

    if (!cJSON_IsNumber(id_utente_json) || !cJSON_IsNumber(id_film_json)) {
        cJSON_Delete(json);
        const char *resp = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    int success = insert_cart_item(id_utente_json->valueint, id_film_json->valueint);
    cJSON_Delete(json);

    if (!success) {
        const char *resp = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 21\r\n\r\n"
            "Errore inserimento carrello";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    // Risposta OK senza contenuto
    const char *resp = 
        "HTTP/1.1 201 Created\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_carrello_options(SOCKET client_socket) {
    const char *resp = 
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
         "Access-Control-Allow-Methods: POST, OPTIONS, DELETE\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Content-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_get_cart_items(SOCKET client_socket, const char *request) {
    printf("Request raw: %s\n", request);


    const char *query = strstr(request, "GET /api/carrello");
    if (!query) return;

    const char *id_param = strstr(query, "id_utente=");
    if (!id_param) return;

    int id_utente = atoi(id_param + strlen("id_utente="));
    if (id_utente <= 0) return;

    char *json = get_cart_items_json(id_utente);

    if (!json) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 22\r\n\r\nErrore nel DB carrello";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n", strlen(json));

    send(client_socket, header, strlen(header), 0);
    send(client_socket, json, strlen(json), 0);
    free(json);
}

void handle_delete_cart_item(SOCKET client_socket, const char *request) {
    // Leggi il body JSON dalla richiesta DELETE (assumendo sia presente)
    const char *body = strstr(request, "\r\n\r\n");
    if (!body) {
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }
    body += 4; // skip \r\n\r\n

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    cJSON *id_utente_json = cJSON_GetObjectItemCaseSensitive(json, "id_utente");
    cJSON *id_film_json = cJSON_GetObjectItemCaseSensitive(json, "id_film");

    if (!cJSON_IsNumber(id_utente_json) || !cJSON_IsNumber(id_film_json)) {
        cJSON_Delete(json);
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    int success = delete_cart_item(id_utente_json->valueint, id_film_json->valueint);
    cJSON_Delete(json);

    if (!success) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 24\r\n\r\n"
            "Errore cancellazione carrello";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}


