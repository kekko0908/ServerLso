#include "DB/film_db.h"
#include <libpq-fe.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

char* get_all_films_json() {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return NULL;
    }

    PGresult *res = PQexec(conn, "SELECT id, titolo, genere, descrizione, copie_disponibili, copie_totali, visualizzazioni, image, prezzo, anno FROM film");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < PQntuples(res); i++) {
        cJSON *film = cJSON_CreateObject();
        cJSON_AddNumberToObject(film, "id", atoi(PQgetvalue(res, i, 0)));
        cJSON_AddStringToObject(film, "titolo", PQgetvalue(res, i, 1));
        cJSON_AddStringToObject(film, "genere", PQgetvalue(res, i, 2));
        cJSON_AddStringToObject(film, "descrizione", PQgetvalue(res, i, 3));
        cJSON_AddNumberToObject(film, "copie_disponibili", atoi(PQgetvalue(res, i, 4)));
        cJSON_AddNumberToObject(film, "copie_totali", atoi(PQgetvalue(res, i, 5)));
        cJSON_AddNumberToObject(film, "visualizzazioni", atoi(PQgetvalue(res, i, 6)));
        cJSON_AddStringToObject(film, "image", PQgetvalue(res, i, 7));
        cJSON_AddNumberToObject(film, "prezzo", atof(PQgetvalue(res, i, 8)));
        cJSON_AddNumberToObject(film, "anno", atoi(PQgetvalue(res, i, 9)));
        cJSON_AddItemToArray(arr, film);
    }

     char *json_str = cJSON_Print(arr); 

    

    cJSON_Delete(arr);
    PQclear(res);
    PQfinish(conn);

    return json_str;
}

char* get_popular_films_json() {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return NULL;
    }

    PGresult *res = PQexec(conn,
        "SELECT id, titolo, genere, descrizione, copie_disponibili, copie_totali, visualizzazioni, image, prezzo, anno "
        "FROM film ORDER BY visualizzazioni DESC LIMIT 7");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < PQntuples(res); i++) {
        cJSON *film = cJSON_CreateObject();
        cJSON_AddNumberToObject(film, "id", atoi(PQgetvalue(res, i, 0)));
        cJSON_AddStringToObject(film, "titolo", PQgetvalue(res, i, 1));
        cJSON_AddStringToObject(film, "genere", PQgetvalue(res, i, 2));
        cJSON_AddStringToObject(film, "descrizione", PQgetvalue(res, i, 3));
        cJSON_AddNumberToObject(film, "copie_disponibili", atoi(PQgetvalue(res, i, 4)));
        cJSON_AddNumberToObject(film, "copie_totali", atoi(PQgetvalue(res, i, 5)));
        cJSON_AddNumberToObject(film, "visualizzazioni", atoi(PQgetvalue(res, i, 6)));
        cJSON_AddStringToObject(film, "image", PQgetvalue(res, i, 7));
        cJSON_AddNumberToObject(film, "prezzo", atof(PQgetvalue(res, i, 8)));
        cJSON_AddNumberToObject(film, "anno", atoi(PQgetvalue(res, i, 9)));
        cJSON_AddItemToArray(arr, film);
    }

    char *json_str = cJSON_Print(arr);
    cJSON_Delete(arr);
    PQclear(res);
    PQfinish(conn);

    return json_str;
}

