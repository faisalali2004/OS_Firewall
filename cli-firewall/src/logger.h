#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>

class Logger {
public:
    Logger() = default;
    ~Logger() = default;
    void log(const std::string& msg) {
        std::ofstream logFile("firewall.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << msg << std::endl;
            logFile.close();
        }
    }
};

#endif // LOGGER_H