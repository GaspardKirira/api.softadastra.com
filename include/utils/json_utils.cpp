#ifndef JSON_UTILS_HPP
#define JSON_UTILS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

namespace json_utils {

    inline nlohmann::json read_json(const std::string &filename) {
        if  (!fs::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename);
        }
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }
        nlohmann::json json_data;
        try {
            file >> json_data;
        } catch (const nlohmann::json::parse_error &e) {
            throw std::runtime_error("JSON parse error in file " + filename + ": " + e.what());
        }
        file.close();
        return json_data;
    }

    inline void write_json(const std::string &filename, const nlohmann::json &json_data) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }
        try {
            file << json_data.dump(4);
        } catch (const std::exception &e) {
            throw std::runtime_error("Error writing JSON to file " + filename + ": " + e.what());
        }
        file.close();
        if (!fs::exists(filename)) {
            throw std::runtime_error("File was not created: " + filename);
        }

        std::cout << "JSON data successfully written to " << filename << std::endl;
    }

    inline bool is_valid_json(const std::string &filename) {
        try {
            read_json(filename);
            return true;
        } catch (const std::runtime_error &) {
            return false;
        }
    }

}

#endif // JSON_UTILS_HPP