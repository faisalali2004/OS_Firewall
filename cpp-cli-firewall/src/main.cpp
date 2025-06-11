#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "firewall.h"
#include "rules.h"

void displayUsage() {
    std::cout << "Usage: cpp-cli-firewall <command> [options]\n"
              << "Commands:\n"
              << "  start [--iface <interface>] [--dpi on|off] [--shape on|off]   Start the firewall\n"
              << "  stop                                                          Stop the firewall\n"
              << "  list                                                          List all rules\n"
              << "  add <id>,<desc>,<action>,<enabled>                            Add a rule\n"
              << "  remove <id>                                                   Remove rule by ID\n"
              << "  showlogs                                                      Show firewall logs\n"
              << "  showpackets                                                   Show captured packets\n"
              << "  clearpackets                                                  Clear captured packets log\n"
              << "  interactive                                                   Enter interactive shell\n"
              << std::endl;
}

void showPackets() {
    std::ifstream log("packets.log");
    if (!log.is_open()) {
        std::cout << "No packets captured yet." << std::endl;
    } else {
        std::cout << "=== Captured Packets ===" << std::endl;
        std::string line;
        while (std::getline(log, line)) {
            std::cout << line << std::endl;
        }
    }
}

void clearPackets() {
    std::ofstream ofs("packets.log", std::ofstream::trunc);
    ofs.close();
    std::cout << "Captured packets log cleared." << std::endl;
}

void interactiveShell(Firewall& firewall) {
    std::cout << "Entering interactive mode. Type 'help' for commands, 'exit' to quit.\n";
    std::string line;
    while (true) {
        std::cout << "firewall> ";
        if (!std::getline(std::cin, line)) break;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "exit" || cmd == "quit") break;
        if (cmd == "help") { displayUsage(); continue; }
        if (cmd == "start") firewall.start();
        else if (cmd == "stop") firewall.stop();
        else if (cmd == "list") firewall.listRules();
        else if (cmd == "add") {
            std::string ruleStr;
            std::getline(iss, ruleStr);
            Rule rule;
            if (rule.deserialize(ruleStr)) {
                firewall.addRule(rule);
            } else {
                std::cerr << "[!] Invalid rule format. Use: id,desc,action,enabled\n";
            }
        }
        else if (cmd == "remove") {
            std::string id; iss >> id;
            if (!id.empty())
                firewall.removeRule(id);
            else
                std::cerr << "[!] Please provide rule ID.\n";
        }
        else if (cmd == "setiface") {
            std::string iface; iss >> iface;
            if (!iface.empty()) firewall.setCaptureInterface(iface);
            else std::cerr << "[!] Please provide interface name.\n";
        }
        else if (cmd == "showlogs") {
            firewall.showLogs();
        }
        else if (cmd == "showpackets") {
            showPackets();
        }
        else if (cmd == "clearpackets") {
            clearPackets();
        }
        else if (cmd == "dpi") {
            std::string opt; iss >> opt;
            firewall.enableDPI(opt == "on");
        }
        else if (cmd == "shape") {
            std::string opt; iss >> opt;
            firewall.enableTrafficShaping(opt == "on");
        }
        else {
            std::cerr << "[!] Unknown command.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    Firewall firewall;
    std::string iface = "any";
    bool dpi = false;
    bool shaping = false;

    if (argc < 2) {
        displayUsage();
        return 1;
    }

    std::string command = argv[1];

    // Parse options for 'start'
    if (command == "start") {
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--iface" && i + 1 < argc) {
                iface = argv[++i];
            } else if (arg == "--dpi" && i + 1 < argc) {
                std::string val = argv[++i];
                dpi = (val == "on");
            } else if (arg == "--shape" && i + 1 < argc) {
                std::string val = argv[++i];
                shaping = (val == "on");
            } else {
                std::cerr << "[!] Unknown option: " << arg << std::endl;
                displayUsage();
                return 1;
            }
        }
        firewall.setCaptureInterface(iface);
        firewall.enableDPI(dpi);
        firewall.enableTrafficShaping(shaping);
        firewall.start();
        return 0;
    }

    if (command == "stop") {
        firewall.stop();
        return 0;
    }

    if (command == "list") {
        firewall.listRules();
        return 0;
    }

    if (command == "add" && argc >= 3) {
        std::string ruleStr = argv[2];
        for (int i = 3; i < argc; ++i) ruleStr += " " + std::string(argv[i]);
        Rule rule;
        if (rule.deserialize(ruleStr)) {
            firewall.addRule(rule);
        } else {
            std::cerr << "[!] Invalid rule format. Use: id,desc,action,enabled\n";
            return 1;
        }
        return 0;
    }

    if (command == "remove" && argc == 3) {
        std::string id = argv[2];
        firewall.removeRule(id);
        return 0;
    }

    if (command == "showlogs") {
        firewall.showLogs();
        return 0;
    }

    if (command == "showpackets") {
        showPackets();
        return 0;
    }

    if (command == "clearpackets") {
        clearPackets();
        return 0;
    }

    if (command == "interactive") {
        interactiveShell(firewall);
        return 0;
    }

    displayUsage();
    return 1;
}