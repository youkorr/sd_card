#pragma once
#include "esphome.h"

namespace esphome {
namespace storage {

class StorageComponent : public Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  void add_file(const std::string &source, const std::string &id) {
    files_.push_back({source, id});
  }
  
  void setup() override {
    if (platform_ == "flash") {
      setup_flash();
    } else if (platform_ == "inline") {
      setup_inline();
    }
  }
  
 private:
  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
  
  void setup_flash() {
    for (const auto &file : files_) {
      ESP_LOGD("storage", "Setting up flash storage: %s -> %s", 
               file.first.c_str(), file.second.c_str());
    }
  }
  
  void setup_inline() {
    for (const auto &file : files_) {
      ESP_LOGD("storage", "Setting up inline storage: %s -> %s", 
               file.first.c_str(), file.second.c_str());
    }
  }
};

}  // namespace storage
}  // namespace esphome
