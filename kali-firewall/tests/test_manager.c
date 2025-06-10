#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "manager.h"

void test_firewall_manager_initialization() {
    FirewallManager *manager = create_firewall_manager();
    assert(manager != NULL);
    assert(manager->is_running == 0);
    destroy_firewall_manager(manager);
}

void test_firewall_manager_start_stop() {
    FirewallManager *manager = create_firewall_manager();
    start_firewall_manager(manager);
    assert(manager->is_running == 1);
    stop_firewall_manager(manager);
    assert(manager->is_running == 0);
    destroy_firewall_manager(manager);
}

void test_firewall_manager_rule_application() {
    FirewallManager *manager = create_firewall_manager();
    Rule rule = { .ip = "192.168.1.1", .port = 80, .protocol = "TCP" };
    add_rule_to_manager(manager, &rule);
    assert(manager->rule_count == 1);
    apply_rules(manager);
    // Add assertions to verify the expected behavior after rule application
    destroy_firewall_manager(manager);
}

int main() {
    test_firewall_manager_initialization();
    test_firewall_manager_start_stop();
    test_firewall_manager_rule_application();
    printf("All tests passed!\n");
    return 0;
}