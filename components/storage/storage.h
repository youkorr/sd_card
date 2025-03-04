#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include <vector>
#include <string>

namespace esphome {
namespace storage {

class StorageComponent : public esphome::Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  
  void add_file(std::function<std::vector<uint8_t>()> source_func, const std::string &id) {
    files_.push_back({source_func, id});
  }
  
  void add_image(const std::string &file, const std::string &id) {
    images_.push_back({file, id});
  }
  
  void play_media(const std::string &media_file);
  void load_image(const std::string &image_id);
  void setup() override;
  void setup_sd_card();

 protected:
  void setup_flash();
  void setup_inline();

 private:
  std::string platform_;
  std::vector<std::pair<std::function<std::vector<uint8_t>()>, std::string>> files_;
  std::vector<std::pair<std::string, std::string>> images_;
};

// Rest of the code remains the same...

}  // namespace storage
}  // namespace esphome















