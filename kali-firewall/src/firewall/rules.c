#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rules.h"

// Function to add a rule
int add_rule(Rule *rules, int *rule_count, const char *ip, int port, const char *protocol) {
    if (*rule_count >= MAX_RULES) {
        return -1; // Rule limit reached
    }
    strncpy(rules[*rule_count].ip, ip, sizeof(rules[*rule_count].ip) - 1);
    rules[*rule_count].port = port;
    strncpy(rules[*rule_count].protocol, protocol, sizeof(rules[*rule_count].protocol) - 1);
    (*rule_count)++;
    return 0; // Success
}

// Function to remove a rule
int remove_rule(Rule *rules, int *rule_count, const char *ip, int port, const char *protocol) {
    for (int i = 0; i < *rule_count; i++) {
        if (strcmp(rules[i].ip, ip) == 0 && rules[i].port == port && strcmp(rules[i].protocol, protocol) == 0) {
            // Shift rules down to remove the rule
            for (int j = i; j < *rule_count - 1; j++) {
                rules[j] = rules[j + 1];
            }
            (*rule_count)--;
            return 0; // Success
        }
    }
    return -1; // Rule not found
}

// Function to evaluate a rule
int evaluate_rule(const Rule *rules, int rule_count, const char *ip, int port, const char *protocol) {
    for (int i = 0; i < rule_count; i++) {
        if (strcmp(rules[i].ip, ip) == 0 && rules[i].port == port && strcmp(rules[i].protocol, protocol) == 0) {
            return 1; // Rule matched
        }
    }
    return 0; // No match
}