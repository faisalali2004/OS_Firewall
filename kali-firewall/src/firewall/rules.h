typedef struct {
    char ip[16];        // IP address in string format
    int port;          // Port number
    char protocol[8];  // Protocol type (e.g., TCP, UDP)
} FirewallRule;

void add_rule(FirewallRule rule);
void remove_rule(FirewallRule rule);
int evaluate_rule(FirewallRule rule);