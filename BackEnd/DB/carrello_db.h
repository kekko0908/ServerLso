#ifndef CARRELLO_DB_H
#define CARRELLO_DB_H

#include <libpq-fe.h>

int insert_cart_item(int id_utente, int id_film);
char *get_cart_items_json(int id_utente);
int delete_cart_item(int id_utente, int id_film);

#endif 
