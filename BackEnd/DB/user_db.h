#pragma once

int insert_user(const char *nome, const char *cognome, const char *email, const char *password);
int check_user_credentials_get_id(const char *email, const char *password, int *user_id_out);
char *get_all_users_json();

