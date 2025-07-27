#include "utils/EnvLoader.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

void EnvLoader::load(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::string line;
    while (std::getline(file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::size_t pos = line.find('=');
        if (pos == std::string::npos) {
            throw std::runtime_error("Invalid line in env file: " + line);
        }
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        // Remove leading and trailing whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        // Set the environment variable
        if (setenv(key.c_str(), value.c_str(), 1) != 0) {
            throw std::runtime_error("Failed to set environment variable: " + key);
        }
    }
    file.close();
}

