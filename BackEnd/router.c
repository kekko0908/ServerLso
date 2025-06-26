#include "router.h"
#include "handlers/user_handler.h"
#include "handlers/film_handler.h"
#include "handlers/carrello_handler.h"
#include "handlers/prestiti_handler.h"
#include <string.h>

void route_request(SOCKET client_socket, const char *request) {
    // CORS preflight per /register
    if (strncmp(request, "OPTIONS /register", 17) == 0) {
        handle_register_options(client_socket);
        return;
    }

    // CORS preflight per /login
    if (strncmp(request, "OPTIONS /login", 14) == 0) {
        handle_login_options(client_socket);
        return;
    }

    // CORS preflight per /logout
    if (strncmp(request, "OPTIONS /api/logout", 19) == 0) {
        handle_logout_options(client_socket);
        return;
    }

    // OPTIONS /api/carrello
if (strncmp(request, "OPTIONS /api/carrello", 21) == 0) {
    handle_carrello_options(client_socket);
    return;
}

if (strncmp(request, "OPTIONS /api/prestiti", 21) == 0) {
    handle_prestiti_options(client_socket, request);
    return;
}

// Aggiungi subito dopo gli altri OPTIONS...

if (strncmp(request, "OPTIONS /api/prestiti", 20) == 0) {
    handle_prestiti_options(client_socket, request);
    return;
}


// POST /api/prestiti
if (strncmp(request, "POST /api/prestiti", 18) == 0) {
    handle_post_prestito(client_socket, request);
    return;
}



    // POST /register
    if (strncmp(request, "POST /register", 14) == 0) {
        handle_register(client_socket, request);
        return;
    }

    // POST /login
    if (strncmp(request, "POST /login", 11) == 0) {
        handle_login(client_socket, request);
        return;
    }

    // POST /api/logout
    if (strncmp(request, "POST /api/logout", 16) == 0) {
        handle_logout(client_socket, request);
        return;
    }

    // POST /api/carrello
if (strncmp(request, "POST /api/carrello", 18) == 0) {
    handle_post_cart_item(client_socket, request);
    return;
}



    // GET /users
    if (strncmp(request, "GET /users", 10) == 0) {
        handle_get_all_users(client_socket);
        return;
    }

    if (strncmp(request, "GET /api/film", 12) == 0) {
    handle_get_all_films(client_socket);
    return;
}

if (strncmp(request, "GET /api/film/popular", 21) == 0) {
    handle_get_popular_films(client_socket);
    return;
}

if (strncmp(request, "GET /api/carrello", 16) == 0) {
    handle_get_cart_items(client_socket, request);
    return;
}

if (strncmp(request, "GET /api/prestiti", 17) == 0) {
    handle_get_prestiti(client_socket, request);
    return;
}

// Gestione PUT /api/prestiti/:id_prestito
if (strncmp(request, "PUT /api/prestiti/", 17) == 0) {
    handle_put_prestito(client_socket, request);
    return;
}


if (strncmp(request, "DELETE /api/carrello", 19) == 0) {
    handle_delete_cart_item(client_socket, request);
    return;
}


   
    // 404 Not Found
    const char *resp =
        "HTTP/1.1 404 Not Found\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 17\r\n\r\nEndpoint mancante";
    send(client_socket, resp, strlen(resp), 0);
}

