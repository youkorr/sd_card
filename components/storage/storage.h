#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include <sys/stat.h>  // Pour file_exists
#include <vector>
#include <string>
#include <utility>  // Pour std::pair

namespace esphome {
namespace storage {

class StorageComponent : public esphome::Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  void add_file(const std::string &source, const std::string &id) {
    files_.push_back({source, id});
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
  bool file_exists(const std::string &path);  // Déclaration de file_exists
  void list_files(const std::string &path);  // Déclaration de list_files

  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
  std::vector<std::pair<std::string, std::string>> images_;
};

// Template class pour PlayMediaAction
template<typename... Ts>
class PlayMediaAction : public esphome::Action<Ts...> {
 public:
  explicit PlayMediaAction(StorageComponent *storage) : storage_(storage) {}
  void set_media_file(const std::string &media_file) { media_file_ = media_file; }
  void play_media() {  // Renommer play en play_media
    if (storage_ && !media_file_.empty()) {
      storage_->play_media(media_file_);
    }
  }

 protected:
  void play() override {  // Implémentation de play pour l'action
    this->play_media();
  }

 private:
  StorageComponent *storage_{nullptr};
  std::string media_file_;
};

// Template class pour LoadImageAction
template<typename... Ts>
class LoadImageAction : public esphome::Action<Ts...> {
 public:
  explicit LoadImageAction(StorageComponent *storage) : storage_(storage) {}
  void set_image_id(const std::string &image_id) { image_id_ = image_id; }
  void load_image() {  // Renommer play en load_image
    if (storage_ && !image_id_.empty()) {
      storage_->load_image(image_id_);
    }
  }

 protected:
  void play() override {  // Implémentation de play pour l'action
    this->load_image();
  }

 private:
  StorageComponent *storage_{nullptr};
  std::string image_id_;
};

}  // namespace storage
}  // namespace esphome












