#ifndef SESSION_H
#define SESSION_H

#include <time.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LENGTH 64
#define EMAIL_LENGTH 256
#define SESSION_EXPIRY_SECONDS (60 * 60 * 24) // esempio: 1 giorno

typedef struct session_entry {
    char session_id[SESSION_ID_LENGTH];
    char email[EMAIL_LENGTH];
    time_t created_at;
    int logged_in;   // 1 se loggato, 0 altrimenti
    int user_id;     // id utente associato
} Session;

void init_sessions();
char *generate_random_session_id(char *buffer, size_t length);
void add_session(const char *session_id, const char *email);
void remove_session(const char *session_id);
const char *get_email_from_session(const char *session_id);
void cleanup_expired_sessions();
Session *getSessionFromRequest(const char *request);

#endif
