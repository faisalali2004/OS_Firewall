#include <stdio.h>
#include "firewall/manager.h"
#include "utils/logger.h"

int main(int argc, char *argv[]) {
    // Initialize the logger
    init_logger();

    // Initialize the firewall manager
    if (init_firewall() != 0) {
        log_error("Failed to initialize the firewall.");
        return 1;
    }

    // Start the main event loop
    log_info("Firewall is running...");
    start_event_loop();

    // Cleanup before exiting
    cleanup_firewall();
    log_info("Firewall has been stopped.");

    return 0;
}