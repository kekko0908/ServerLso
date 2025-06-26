#ifndef PRESTITI_DB_H
#define PRESTITI_DB_H

#include <cJSON.h>

int insert_prestito(int id_utente, int id_film, const char *data_restituzione_prevista) ;
cJSON *get_prestiti_by_utente(int id_utente);
int update_restituzione_effettiva(int id_prestito, const char *data_restituzione_effettiva);
int decrementa_copie_disponibili(int id_film);
int incrementa_copie_disponibili(int id_film);

#endif
