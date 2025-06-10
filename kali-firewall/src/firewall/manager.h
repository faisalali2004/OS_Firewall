#ifndef MANAGER_H
#define MANAGER_H

#include "rules.h"

// Function prototypes for managing firewall operations
void initialize_firewall();
void start_packet_capture();
void process_packet(const char *packet);
void apply_rules(const Rule *rules, int rule_count);
void stop_firewall();

#endif // MANAGER_H