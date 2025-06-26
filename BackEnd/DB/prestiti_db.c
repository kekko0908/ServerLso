#include "DB/prestiti_db.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "cJSON.h"
#include <libpq-fe.h>
#include <pthread.h>

static pthread_mutex_t prestiti_mutex = PTHREAD_MUTEX_INITIALIZER;


int insert_prestito(int id_utente, int id_film, const char *data_restituzione_prevista) {
    pthread_mutex_lock(&prestiti_mutex);
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connessione al DB fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    const char *query = "INSERT INTO prestiti (id_utente, id_film, data_prestito, data_restituzione_prevista, data_restituzione_effettiva) "
                        "VALUES ($1, $2, CURRENT_DATE, $3, NULL)";

    char id_utente_str[12], id_film_str[12];
    snprintf(id_utente_str, sizeof(id_utente_str), "%d", id_utente);
    snprintf(id_film_str, sizeof(id_film_str), "%d", id_film);

    const char *params[3] = { id_utente_str, id_film_str, data_restituzione_prevista };

    PGresult *res = PQexecParams(conn, query, 3, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore inserimento prestito: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    PQclear(res);

    // Decrementa le copie disponibili del film
    if (!decrementa_copie_disponibili(id_film)) {
        fprintf(stderr, "Errore decremento copie film\n");
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    PQfinish(conn);
    pthread_mutex_unlock(&prestiti_mutex);
    return 1;
}




cJSON *get_prestiti_by_utente(int id_utente) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connessione fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    const char *query =
        "SELECT p.id, f.titolo, f.image, f.prezzo, p.data_prestito, p.data_restituzione_prevista "
        "FROM prestiti p "
        "JOIN film f ON p.id_film = f.id "
        "WHERE p.id_utente = $1 AND p.data_restituzione_effettiva IS NULL";

    char id_utente_str[12];
    snprintf(id_utente_str, sizeof(id_utente_str), "%d", id_utente);
    const char *params[1] = { id_utente_str };

    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Errore nella SELECT: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    int rows = PQntuples(res);
    cJSON *array = cJSON_CreateArray();

    for (int i = 0; i < rows; ++i) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id_prestito", atoi(PQgetvalue(res, i, 0)));
    cJSON_AddStringToObject(item, "titolo", PQgetvalue(res, i, 1));
    cJSON_AddStringToObject(item, "img", PQgetvalue(res, i, 2));
    cJSON_AddNumberToObject(item, "prezzo", atof(PQgetvalue(res, i, 3)));
    cJSON_AddStringToObject(item, "data_prestito", PQgetvalue(res, i, 4));
    cJSON_AddStringToObject(item, "data_restituzione_prevista", PQgetvalue(res, i, 5));
        cJSON_AddItemToArray(array, item);
    }

    PQclear(res);
    PQfinish(conn);
    return array;
}

int update_restituzione_effettiva(int id_prestito, const char *data_restituzione_effettiva) {
    pthread_mutex_lock(&prestiti_mutex);
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connessione al DB fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    // 1. Recupera id_film associato al prestito
    const char *select_query = "SELECT id_film FROM prestiti WHERE id = $1";
    char id_prestito_str[12];
    snprintf(id_prestito_str, sizeof(id_prestito_str), "%d", id_prestito);
    const char *select_params[1] = { id_prestito_str };

    PGresult *select_res = PQexecParams(conn, select_query, 1, NULL, select_params, NULL, NULL, 0);
    if (PQresultStatus(select_res) != PGRES_TUPLES_OK || PQntuples(select_res) != 1) {
        fprintf(stderr, "Errore recupero id_film: %s\n", PQresultErrorMessage(select_res));
        PQclear(select_res);
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    int id_film = atoi(PQgetvalue(select_res, 0, 0));
    PQclear(select_res);

    // 2. Aggiorna data restituzione
    const char *update_query = "UPDATE prestiti SET data_restituzione_effettiva = $1 WHERE id = $2";
    const char *update_params[2] = { data_restituzione_effettiva, id_prestito_str };

    PGresult *update_res = PQexecParams(conn, update_query, 2, NULL, update_params, NULL, NULL, 0);
    if (PQresultStatus(update_res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore aggiornamento restituzione: %s\n", PQresultErrorMessage(update_res));
        PQclear(update_res);
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }
    PQclear(update_res);

    // 3. Incrementa copie disponibili
    if (!incrementa_copie_disponibili(id_film)) {
        fprintf(stderr, "Errore incremento copie disponibili\n");
        PQfinish(conn);
        pthread_mutex_unlock(&prestiti_mutex);
        return 0;
    }

    PQfinish(conn);
    pthread_mutex_unlock(&prestiti_mutex);
    return 1;
}

int decrementa_copie_disponibili(int id_film) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connessione al DB fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 0;
    }

    const char *query = "UPDATE film SET copie_disponibili = copie_disponibili - 1 WHERE id = $1 AND copie_disponibili > 0";

    char id_film_str[12];
    snprintf(id_film_str, sizeof(id_film_str), "%d", id_film);

    const char *params[1] = { id_film_str };

    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore decremento copie: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        return 0;
    }

    // Controlla righe aggiornate
    int affected_rows = atoi(PQcmdTuples(res));
    PQclear(res);
    PQfinish(conn);

    if (affected_rows == 0) {
        // Nessuna copia disponibile da decrementare
        return 0;
    }
    return 1;
}


int incrementa_copie_disponibili(int id_film) {
    PGconn *conn = PQconnectdb("host=localhost dbname=FGStudios user=postgres password=postgres");
    PQsetClientEncoding(conn, "UTF8");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connessione al DB fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 0;
    }

    const char *query = "UPDATE film SET copie_disponibili = copie_disponibili + 1 WHERE id = $1";

    char id_film_str[12];
    snprintf(id_film_str, sizeof(id_film_str), "%d", id_film);
    const char *params[1] = { id_film_str };

    PGresult *res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore incremento copie: %s\n", PQresultErrorMessage(res));
        PQclear(res);
        PQfinish(conn);
        return 0;
    }

    PQclear(res);
    PQfinish(conn);
    return 1;
}









