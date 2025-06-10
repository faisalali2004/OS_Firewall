#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

void log_event(const char *message) {
    FILE *log_file = fopen("firewall.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline character

    fprintf(log_file, "[%s] %s\n", timestamp, message);
    fclose(log_file);
}

void log_error(const char *message) {
    log_event(message);
}

void log_info(const char *message) {
    log_event(message);
}