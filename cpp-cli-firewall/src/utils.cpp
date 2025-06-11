#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>

std::string currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &now);
#else
    localtime_r(&now, &tm_now);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void logMessage(const std::string& message, LogLevel level) {
    std::ofstream logFile("firewall.log", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << currentTimestamp() << "] ";
        switch (level) {
            case LogLevel::INFO: logFile << "[INFO] "; break;
            case LogLevel::WARNING: logFile << "[WARNING] "; break;
            case LogLevel::ERROR: logFile << "[ERROR] "; break;
        }
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "[ERROR] Unable to open log file." << std::endl;
    }
}

void handleError(const std::string& errorMessage, bool fatal) {
    std::cerr << "[ERROR] " << errorMessage << std::endl;
    logMessage(errorMessage, LogLevel::ERROR);
    if (fatal) std::exit(EXIT_FAILURE);
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) start++;
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}