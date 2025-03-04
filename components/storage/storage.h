#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <vector>
#include <string>
#include <map>

namespace esphome {
namespace storage {

class MediaPlayerComponent : public esphome::Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  
  void add_file(std::function<std::vector<uint8_t>()> source_func, const std::string &id) {
    files_[id] = source_func;
  }
  
  std::vector<uint8_t> read_file(const std::string &id) {
    auto it = files_.find(id);
    if (it != files_.end()) {
      return it->second();
    }
    return {};
  }
  
  void play_file(const std::string &id);
  void stop_playback();
  
  void setup() override;
  void loop() override;

  bool is_playing() const { return is_playing_; }
  std::string current_file() const { return current_file_id_; }

 private:
  std::string platform_;
  std::map<std::string, std::function<std::vector<uint8_t>()>> files_;
  
  // Playback state
  std::string current_file_id_;
  bool is_playing_ = false;
};

}  // namespace storage
}  // namespace esphome















