#include "Session/session.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct session_entry sessions[MAX_SESSIONS];

void init_sessions() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        sessions[i].session_id[0] = '\0';
        sessions[i].email[0] = '\0';
        sessions[i].created_at = 0;
    }
}

char *generate_random_session_id(char *buffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;
    srand((unsigned int)time(NULL) ^ rand());

    for (size_t i = 0; i < length - 1; i++) {
        buffer[i] = charset[rand() % charset_size];
    }
    buffer[length - 1] = '\0';
    return buffer;
}

void add_session(const char *session_id, const char *email) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].session_id[0] == '\0') {
            strncpy(sessions[i].session_id, session_id, SESSION_ID_LENGTH);
            strncpy(sessions[i].email, email, sizeof(sessions[i].email));
            sessions[i].created_at = time(NULL);
            return;
        }
    }
    
    printf("WARNING: Max sessions reached!\n");
}

void remove_session(const char *session_id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            sessions[i].session_id[0] = '\0';
            sessions[i].email[0] = '\0';
            sessions[i].created_at = 0;
            return;
        }
    }
}

const char *get_email_from_session(const char *session_id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            // Check se scaduta
            if (time(NULL) - sessions[i].created_at > SESSION_EXPIRY_SECONDS) {
                // Sessione scaduta → rimuovi
                remove_session(session_id);
                return NULL;
            }
            return sessions[i].email;
        }
    }
    return NULL;
}

void cleanup_expired_sessions() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].session_id[0] != '\0' &&
            time(NULL) - sessions[i].created_at > SESSION_EXPIRY_SECONDS) {
            remove_session(sessions[i].session_id);
        }
    }
}

Session *getSessionFromRequest(const char *request) {
    const char *cookie_header = strstr(request, "Cookie:");
    if (!cookie_header) return NULL;

    const char *session_key = strstr(cookie_header, "session_id=");
    if (!session_key) return NULL;

    char session_id[SESSION_ID_LENGTH];
    int i = 0;

    session_key += strlen("session_id=");
    while (*session_key && *session_key != ';' && *session_key != '\r' && i < SESSION_ID_LENGTH - 1) {
        session_id[i++] = *session_key++;
    }
    session_id[i] = '\0';

    // Cerca la sessione
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            if (time(NULL) - sessions[i].created_at > SESSION_EXPIRY_SECONDS) {
                // sessione scaduta
                remove_session(session_id);
                return NULL;
            }
            return &sessions[i];
        }
    }

    return NULL;
}

