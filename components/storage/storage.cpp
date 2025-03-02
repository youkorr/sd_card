#include "storage.h"
#include <iostream>

Storage::Storage(const std::string& platform, const std::vector<std::map<std::string, std::string>>& files)
    : platform(platform), files(files) {}

void Storage::initialize() {
    if (platform == "flash") {
        setup_flash();
    } else if (platform == "inline") {
        setup_inline();
    }
}

void Storage::setup_flash() {
    for (const auto& file : files) {
        std::cout << "Initializing flash storage for file: " 
                  << file.at("source") << " with id: " 
                  << file.at("id") << std::endl;
    }
}

void Storage::setup_inline() {
    for (const auto& file : files) {
        std::cout << "Initializing inline storage for file: " 
                  << file.at("source") << " with id: " 
                  << file.at("id") << std::endl;
    }
}
