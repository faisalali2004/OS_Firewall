#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cctype>

// Log levels for logging
enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// Timestamp utility
std::string currentTimestamp();

// Logging utility with log level
void logMessage(const std::string& message, LogLevel level = LogLevel::INFO);

// Error handling utility
void handleError(const std::string& errorMessage, bool fatal = false);

// File existence check
bool fileExists(const std::string& filename);

// String utilities
std::vector<std::string> splitString(const std::string& str, char delimiter);
std::string trim(const std::string& s);

#endif // UTILS_H