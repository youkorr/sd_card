#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <vector>
#include <string>

namespace esphome {
namespace storage {

class SdCardPlayerComponent : public esphome::Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  
  void add_file(std::function<std::vector<uint8_t>()> source_func, const std::string &id) {
    files_.push_back({source_func, id});
  }
  
  std::vector<uint8_t> read_file(const std::string &id) {
    for (const auto &file : files_) {
      if (file.second == id) {
        return file.first();
      }
    }
    return {};
  }
  
  void play_file(const std::string &id);
  void stop_playback();
  
  void setup() override;
  void loop() override;

 private:
  std::string platform_;
  std::vector<std::pair<std::function<std::vector<uint8_t>()>, std::string>> files_;
  
  // Playback state
  std::string current_file_id_;
  bool is_playing_ = false;
};

}  // namespace storage
}  // namespace esphome















