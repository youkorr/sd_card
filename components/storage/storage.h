#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
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
  void add_image(const std::string &file, const std::string &id) {
    images_.emplace_back(file, id);
  }
  void setup() override;
  
  // New method for media player integration
  std::string get_file_path(const std::string &file_id) const;

 protected:
  void setup_sd_card();
  void setup_flash();
  void setup_inline();

 private:
  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
  std::vector<std::pair<std::string, std::string>> images_;
};

template<typename... Ts>
class PlayMediaAction : public Action<Ts...> {
 public:
  PlayMediaAction(StorageComponent *storage) : storage_(storage) {}
  
  TEMPLATABLE_VALUE(std::string, media_file)
  TEMPLATABLE_VALUE(bool, announcement)
  TEMPLATABLE_VALUE(bool, enqueue)

  void play(Ts... x) override {
    if (storage_) {
      auto file_path = storage_->get_file_path(this->media_file_.value(x...));
      // Implementation to play the file would go here
      // This is where you'd integrate with your media player
    }
  }

 protected:
  StorageComponent *storage_;
};

}  // namespace storage
}  // namespace esphome












