#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

#define LOG_FILE "firewall.log"

void log_event(const char *event);
void log_error(const char *error);
void log_info(const char *info);

#endif // LOGGER_H