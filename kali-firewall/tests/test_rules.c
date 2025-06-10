#include <stdio.h>
#include <assert.h>
#include "rules.h"

void test_add_rule() {
    Rule rule;
    rule.ip = "192.168.1.1";
    rule.port = 80;
    rule.protocol = "TCP";

    assert(add_rule(rule) == 0); // Assuming 0 indicates success
}

void test_remove_rule() {
    Rule rule;
    rule.ip = "192.168.1.1";
    rule.port = 80;
    rule.protocol = "TCP";

    add_rule(rule);
    assert(remove_rule(rule) == 0); // Assuming 0 indicates success
}

void test_evaluate_rule() {
    Rule rule;
    rule.ip = "192.168.1.1";
    rule.port = 80;
    rule.protocol = "TCP";

    add_rule(rule);
    assert(evaluate_rule("192.168.1.1", 80, "TCP") == 1); // Assuming 1 indicates rule matched
    assert(evaluate_rule("192.168.1.2", 80, "TCP") == 0); // Assuming 0 indicates no match
}

int main() {
    test_add_rule();
    test_remove_rule();
    test_evaluate_rule();

    printf("All tests passed!\n");
    return 0;
}