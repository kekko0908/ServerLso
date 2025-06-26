#include "handlers/prestiti_handler.h"
#include "DB/prestiti_db.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cJSON.h"

void handle_post_prestito(SOCKET client_socket, const char *request) {
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
    cJSON *data_restituzione_prevista_json = cJSON_GetObjectItemCaseSensitive(json, "data_restituzione_prevista");

    if (!cJSON_IsNumber(id_utente_json) || !cJSON_IsNumber(id_film_json) || !cJSON_IsString(data_restituzione_prevista_json)) {
        cJSON_Delete(json);
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    int success = insert_prestito(id_utente_json->valueint, id_film_json->valueint, data_restituzione_prevista_json->valuestring);
    cJSON_Delete(json);

    if (!success) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 24\r\n\r\n"
            "Errore inserimento prestito";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    const char *resp =
        "HTTP/1.1 201 Created\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_prestiti_options(SOCKET client_socket, const char *request) {
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
       "Access-Control-Allow-Methods: POST, PUT, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Content-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_get_prestiti(SOCKET client_socket, const char *request) {
    const char *id_param = strstr(request, "id_utente=");
    if (!id_param) {
        const char *resp =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 11\r\n\r\n"
            "Bad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    int id_utente = atoi(id_param + strlen("id_utente="));

    cJSON *prestiti = get_prestiti_by_utente(id_utente);
    if (!prestiti) {
        const char *resp =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 26\r\n\r\nErrore lettura prestiti";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    char *json_str = cJSON_PrintUnformatted(prestiti);
    cJSON_Delete(prestiti);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n",
        strlen(json_str));

    send(client_socket, header, strlen(header), 0);
    send(client_socket, json_str, strlen(json_str), 0);
    free(json_str);
}

void handle_put_prestito(SOCKET client_socket, const char *request) {
    // Estrai ID prestito dall'URL: /api/prestiti/:id
    const char *start = strstr(request, "/api/prestiti/");
    if (!start) {
        // URL malformato
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 11\r\n\r\nBad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }
    start += strlen("/api/prestiti/");

    // estrai l'id prestito (fino a spazio o nuova linea)
    char id_buf[16] = {0};
    int i = 0;
    while (start[i] && start[i] != ' ' && start[i] != '\r' && start[i] != '\n' && i < 15) {
        id_buf[i] = start[i];
        i++;
    }
    int id_prestito = atoi(id_buf);
    if (id_prestito <= 0) {
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 11\r\n\r\nBad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    // Trova il body (dopo \r\n\r\n)
    const char *body = strstr(request, "\r\n\r\n");
    if (!body) {
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 11\r\n\r\nBad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }
    body += 4;

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 11\r\n\r\nBad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    cJSON *data_restituzione_effettiva_json = cJSON_GetObjectItemCaseSensitive(json, "data_restituzione_effettiva");
    if (!cJSON_IsString(data_restituzione_effettiva_json)) {
        cJSON_Delete(json);
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 11\r\n\r\nBad Request";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    int success = update_restituzione_effettiva(id_prestito, data_restituzione_effettiva_json->valuestring);
    cJSON_Delete(json);

    if (!success) {
        const char *resp = "HTTP/1.1 500 Internal Server Error\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 25\r\n\r\nErrore aggiornamento prestito";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    const char *resp = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}


