#include <iostream>
#include <string>
#include <sstream>
#include "firewall.h"

void displayUsage() {
    std::cout << "Usage: cpp-cli-firewall <command> [options]\n"
              << "Commands:\n"
              << "  start                Start the firewall\n"
              << "  stop                 Stop the firewall\n"
              << "  status               Show firewall status\n"
              << "  list                 List all rules\n"
              << "  add <rule>           Add a rule (format: id,desc,enabled)\n"
              << "  remove <id|index>    Remove rule by ID or index\n"
              << "  enable <id|index>    Enable rule by ID or index\n"
              << "  disable <id|index>   Disable rule by ID or index\n"
              << "  edit <id> <rule>     Edit rule by ID (format: id,desc,enabled)\n"
              << "  interactive          Enter interactive shell\n"
              << std::endl;
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
        else if (cmd == "status") firewall.status();
        else if (cmd == "list") firewall.listRules();
        else if (cmd == "add") {
            std::string ruleStr;
            std::getline(iss, ruleStr);
            Rule rule;
            if (rule.deserialize(ruleStr)) {
                firewall.addRule(rule);
            } else {
                std::cerr << "[!] Invalid rule format.\n";
            }
        }
        else if (cmd == "remove") {
            std::string arg; iss >> arg;
            if (isdigit(arg[0]))
                firewall.removeRuleByIndex(std::stoi(arg));
            else
                firewall.removeRuleById(arg);
        }
        else if (cmd == "enable") {
            std::string arg; iss >> arg;
            if (isdigit(arg[0]))
                firewall.enableRuleByIndex(std::stoi(arg));
            else
                firewall.enableRuleById(arg);
        }
        else if (cmd == "disable") {
            std::string arg; iss >> arg;
            if (isdigit(arg[0]))
                firewall.disableRuleByIndex(std::stoi(arg));
            else
                firewall.disableRuleById(arg);
        }
        else if (cmd == "edit") {
            std::string id, ruleStr;
            iss >> id;
            std::getline(iss, ruleStr);
            Rule rule;
            if (rule.deserialize(ruleStr)) {
                firewall.editRule(id, rule);
            } else {
                std::cerr << "[!] Invalid rule format.\n";
            }
        }
        else {
            std::cerr << "[!] Unknown command.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    Firewall firewall;

    if (argc < 2) {
        displayUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "start") {
        firewall.start();
    } else if (command == "stop") {
        firewall.stop();
    } else if (command == "status") {
        firewall.status();
    } else if (command == "list") {
        firewall.listRules();
    } else if (command == "add" && argc >= 3) {
        Rule rule;
        std::string ruleStr = argv[2];
        for (int i = 3; i < argc; ++i) ruleStr += " " + std::string(argv[i]);
        if (rule.deserialize(ruleStr)) {
            firewall.addRule(rule);
        } else {
            std::cerr << "[!] Invalid rule format.\n";
            return 1;
        }
    } else if (command == "remove" && argc == 3) {
        std::string arg = argv[2];
        if (isdigit(arg[0]))
            firewall.removeRuleByIndex(std::stoi(arg));
        else
            firewall.removeRuleById(arg);
    } else if (command == "enable" && argc == 3) {
        std::string arg = argv[2];
        if (isdigit(arg[0]))
            firewall.enableRuleByIndex(std::stoi(arg));
        else
            firewall.enableRuleById(arg);
    } else if (command == "disable" && argc == 3) {
        std::string arg = argv[2];
        if (isdigit(arg[0]))
            firewall.disableRuleByIndex(std::stoi(arg));
        else
            firewall.disableRuleById(arg);
    } else if (command == "edit" && argc >= 4) {
        std::string id = argv[2];
        std::string ruleStr = argv[3];
        for (int i = 4; i < argc; ++i) ruleStr += " " + std::string(argv[i]);
        Rule rule;
        if (rule.deserialize(ruleStr)) {
            firewall.editRule(id, rule);
        } else {
            std::cerr << "[!] Invalid rule format.\n";
            return 1;
        }
    } else if (command == "interactive") {
        interactiveShell(firewall);
    } else {
        displayUsage();
        return 1;
    }

    return 0;
}