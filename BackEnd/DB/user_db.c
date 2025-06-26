#include "DB/user_db.h"
#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

int insert_user(const char *nome, const char *cognome, const char *email, const char *password) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) return 1;

    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO utenti (nome, cognome, email, password) VALUES ('%s', '%s', '%s', '%s')",
        nome, cognome, email, password);

    PGresult *res = PQexec(conn, query);
    int success = PQresultStatus(res) == PGRES_COMMAND_OK ? 0 : 1;

    PQclear(res);
    PQfinish(conn);
    return success;
}

int check_user_credentials_get_id(const char *email, const char *password, int *user_id_out) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) return 0;

    char query[512];
    

    snprintf(query, sizeof(query),
        "SELECT id FROM utenti WHERE email = '%s' AND password = '%s'", email, password);
        

    PGresult *res = PQexec(conn, query);
    int found = PQntuples(res) > 0;

    if (found) {
        *user_id_out = atoi(PQgetvalue(res, 0, 0)); 
    }

    PQclear(res);
    PQfinish(conn);
    return found;
}

char *get_all_users_json() {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) return NULL;

    PGresult *res = PQexec(conn, "SELECT nome, cognome, email FROM utenti");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < PQntuples(res); i++) {
        cJSON *user = cJSON_CreateObject();
        cJSON_AddStringToObject(user, "nome", PQgetvalue(res, i, 0));
        cJSON_AddStringToObject(user, "cognome", PQgetvalue(res, i, 1));
        cJSON_AddStringToObject(user, "email", PQgetvalue(res, i, 2));
        cJSON_AddItemToArray(arr, user);
    }

    char *json_str = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    PQclear(res);
    PQfinish(conn);

    return json_str;
}
