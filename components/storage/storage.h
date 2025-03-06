#pragma once

#include "esphome/core/component.h"
#include <vector>
#include <string>

namespace esphome {
namespace storage {

class StorageComponent : public Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  void add_file(const std::string &source, const std::string &id) {
    files_.emplace_back(source, id);
  }
  std::string get_file_path(const std::string &id) const {
    for (const auto &file : files_) {
      if (file.second == id) {
        return file.first;
      }
    }
    return "";
  }
  void setup() override;

 protected:
  void setup_sd_card();
  void setup_flash();
  void setup_inline();

 private:
  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
};

}  // namespace storage
}  // namespace esphome












