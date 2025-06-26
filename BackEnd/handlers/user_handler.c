#include "handlers/user_handler.h"
#include "DB/user_db.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include "Session/session.h"

void handle_register_options(SOCKET client_socket) {
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_register(SOCKET client_socket, const char *request) {
    char *body = strstr(request, "\r\n\r\n");
    if (body) body += 4; else body = "";

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        const char *resp = "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\nContent-Length: 19\r\n\r\nErrore nel JSON";
        send(client_socket, resp, strlen(resp), 0);
        return;
    }

    const char *nome = cJSON_GetObjectItem(json, "nome")->valuestring;
    const char *cognome = cJSON_GetObjectItem(json, "cognome")->valuestring;
    const char *email = cJSON_GetObjectItem(json, "email")->valuestring;
    const char *password = cJSON_GetObjectItem(json, "password")->valuestring;

    int res = insert_user(nome, cognome, email, password);

    if (res != 0) {
        const char *resp = "HTTP/1.1 500 Internal Server Error\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\nContent-Length: 23\r\n\r\nErrore nel salvataggio";
        send(client_socket, resp, strlen(resp), 0);
    } else {
        const char *success =
            "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Length: 35\r\n\r\n{\"message\":\"Registrazione OK\"}";
        send(client_socket, success, strlen(success), 0);
    }

    cJSON_Delete(json);
}

void handle_user_request(SOCKET client_socket, const char *method, const char *path, const char *body) {
    if (strcmp(method, "POST") == 0 && strcmp(path, "/register") == 0) {
        handle_register(client_socket, body);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/login") == 0) {
        handle_login(client_socket, body);
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/utenti") == 0) {
        handle_get_all_users(client_socket);
    } else {
        const char *resp =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 13\r\n\r\nEndpoint mancante";
        send(client_socket, resp, strlen(resp), 0);
    }
}

void handle_login(SOCKET client_socket, const char *request) {
    const char *body = strstr(request, "\r\n\r\n");
    if (body) body += 4;
    else body = "";

    cJSON *json = cJSON_Parse(body);
    if (!json) {
        const char *err =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 19\r\n\r\nErrore nel JSON";
        send(client_socket, err, strlen(err), 0);
        return;
    }

    const char *email = cJSON_GetObjectItem(json, "email")->valuestring;
    const char *password = cJSON_GetObjectItem(json, "password")->valuestring;

    int user_id;
    if (check_user_credentials_get_id(email, password, &user_id)) {
        char json_body[512];
        snprintf(json_body, sizeof(json_body), "{\"email\":\"%s\", \"userId\": %d}", email, user_id);
        size_t content_length = strlen(json_body);

        char session_id[SESSION_ID_LENGTH];
        generate_random_session_id(session_id, SESSION_ID_LENGTH);

        add_session(session_id, email);

        char resp[1024];
        int resp_len = snprintf(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Set-Cookie: session=%s; Path=/; HttpOnly; SameSite=Strict\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            session_id, content_length, json_body);

        int sent = 0;
        while (sent < resp_len) {
            int n = send(client_socket, resp + sent, resp_len - sent, 0);
            if (n <= 0) break;
            sent += n;
        }
    } else {
        const char *err =
            "HTTP/1.1 401 Unauthorized\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 18\r\n\r\nCredenziali errate";
        send(client_socket, err, strlen(err), 0);
    }

    cJSON_Delete(json);
}


void handle_get_all_users(SOCKET client_socket) {
    char *json = get_all_users_json();
    if (!json) {
        const char *err =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 22\r\n\r\nErrore nel database";
        send(client_socket, err, strlen(err), 0);
        return;
    }

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %zu\r\n\r\n", strlen(json));

    send(client_socket, header, strlen(header), 0);
    send(client_socket, json, strlen(json), 0);

    free(json);
}

void handle_login_options(SOCKET client_socket) {
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: http://127.0.0.1:3000\r\n"
        "Access-Control-Allow-Credentials: true\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n\r\n";
    send(client_socket, resp, strlen(resp), 0);
}

void handle_logout(SOCKET client_socket, const char *request) {
    // Cerca header Cookie
    const char *cookie_header = strstr(request, "Cookie:");
    char session_id[SESSION_ID_LENGTH] = "";

    if (cookie_header) {
        const char *session_pos = strstr(cookie_header, "session=");
        if (session_pos) {
            session_pos += strlen("session=");
            int i = 0;
            while (*session_pos && *session_pos != ';' && i < SESSION_ID_LENGTH - 1) {
                session_id[i++] = *session_pos++;
            }
            session_id[i] = '\0';
        }
    }

    // Se abbiamo trovato la sessione, la rimuoviamo
    if (session_id[0] != '\0') {
        remove_session(session_id);
    }

    // Corpo della risposta JSON
    const char *body = "{\"message\":\"Logout ok\"}";
    size_t content_length = strlen(body);

    // Risposta HTTP completa con CORS corretto
    char resp[512];
    int resp_len = snprintf(resp, sizeof(resp),
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: http://127.0.0.1:3000\r\n"        // Dominio ESATTO
        "Access-Control-Allow-Credentials: true\r\n"                    // Permetti credenziali
        "Set-Cookie: session=; Path=/; HttpOnly; Max-Age=0; SameSite=Strict\r\n" // Rimuove il cookie
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n%s",
        content_length, body);

    // Invio risposta completa
    int sent = 0;
    while (sent < resp_len) {
        int n = send(client_socket, resp + sent, resp_len - sent, 0);
        if (n <= 0) break;
        sent += n;
    }
}


void handle_logout_options(SOCKET client_socket) {
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: http://127.0.0.1:3000\r\n"  // CORS specifico
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 86400\r\n"
        "Content-Length: 0\r\n\r\n";

    send(client_socket, resp, strlen(resp), 0);
}


