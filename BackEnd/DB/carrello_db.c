#include "DB/carrello_db.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "cJSON.h"
#include <pthread.h>

static pthread_mutex_t carrello_mutex = PTHREAD_MUTEX_INITIALIZER;

int insert_cart_item(int id_utente, int id_film) {
    pthread_mutex_lock(&carrello_mutex);
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 0; 
    }

    // Controlla se già presente
    char check_query[] = "SELECT COUNT(*) FROM carrello WHERE id_utente=$1 AND id_film=$2";
    const char *check_params[2];
    char id_utente_str[12], id_film_str[12];
    snprintf(id_utente_str, sizeof(id_utente_str), "%d", id_utente);
    snprintf(id_film_str, sizeof(id_film_str), "%d", id_film);
    check_params[0] = id_utente_str;
    check_params[1] = id_film_str;

    PGresult *check_res = PQexecParams(conn, check_query, 2, NULL, check_params, NULL, NULL, 0);
    if (PQresultStatus(check_res) != PGRES_TUPLES_OK) {
        PQclear(check_res);
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 0;
    }

    int count = atoi(PQgetvalue(check_res, 0, 0));
    PQclear(check_res);

    if (count > 0) {
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 1; // già presente, considera come successo
    }

    // Inserimento se non presente
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char data_aggiunta[20];
    strftime(data_aggiunta, sizeof(data_aggiunta), "%Y-%m-%d %H:%M:%S", tm_now);

    const char *query = "INSERT INTO carrello (id_utente, id_film, data_aggiunta) VALUES ($1, $2, $3)";
    const char *paramValues[3] = { id_utente_str, id_film_str, data_aggiunta };

    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 0; 
    }

    PQclear(res);
    PQfinish(conn);
    pthread_mutex_unlock(&carrello_mutex);
    return 1;
}

char *get_cart_items_json(int id_utente) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return NULL;
    }

    char id_str[12];
    snprintf(id_str, sizeof(id_str), "%d", id_utente);
   

    const char *params[1] = { id_str };

    const char *query = 
        "SELECT f.id, f.titolo, f.prezzo, f.image "
        "FROM carrello c JOIN film f ON c.id_film = f.id "
        "WHERE c.id_utente = $1";

    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    int rows = PQntuples(res);
    cJSON *json_array = cJSON_CreateArray();

    for (int i = 0; i < rows; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id_film", atoi(PQgetvalue(res, i, 0)));
        cJSON_AddStringToObject(item, "titolo", PQgetvalue(res, i, 1));
        cJSON_AddNumberToObject(item, "prezzo", atof(PQgetvalue(res, i, 2)));
        cJSON_AddStringToObject(item, "img", PQgetvalue(res, i, 3));
        cJSON_AddItemToArray(json_array, item);
    }

    char *json_string = cJSON_PrintUnformatted(json_array);
    cJSON_Delete(json_array);
    PQclear(res);
    PQfinish(conn);
    return json_string;
}

int delete_cart_item(int id_utente, int id_film) {
    pthread_mutex_lock(&carrello_mutex);
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 0;
    }

    char id_utente_str[12], id_film_str[12];
    snprintf(id_utente_str, sizeof(id_utente_str), "%d", id_utente);
    snprintf(id_film_str, sizeof(id_film_str), "%d", id_film);

    const char *params[2] = { id_utente_str, id_film_str };
    const char *query = "DELETE FROM carrello WHERE id_utente = $1 AND id_film = $2";

    PGresult *res = PQexecParams(conn, query, 2, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        PQfinish(conn);
        pthread_mutex_unlock(&carrello_mutex);
        return 0;
    }

    PQclear(res);
    PQfinish(conn);
    pthread_mutex_unlock(&carrello_mutex);
    return 1;
}


