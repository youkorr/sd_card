#pragma once
#include "esphome.h"
#include "esphome/core/action.h"  // Ajout de l'inclusion nécessaire

namespace esphome {
namespace storage {

class StorageComponent : public Component {
 public:
  void set_platform(const std::string &platform) { platform_ = platform; }
  void add_file(const std::string &source, const std::string &id) {
    files_.push_back({source, id});
  }

  void add_image(const std::string &file, const std::string &id) {
    images_.push_back({file, id});
  }

  void play_file(const std::string &file_id) {
    for (const auto &file : files_) {
      if (file.second == file_id) {
        ESP_LOGD("storage", "Playing file: %s", file.first.c_str());
        break;
      }
    }
  }

  void load_image(const std::string &image_id) {
    for (const auto &image : images_) {
      if (image.second == image_id) {
        ESP_LOGD("storage", "Loading image: %s", image.first.c_str());
        break;
      }
    }
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
  std::vector<std::pair<std::string, std::string>> images_;

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

// Correction de l'héritage en s'assurant que `Action` est bien accessible
class PlayAudioFileAction : public esphome::core::Action {
 public:
  void set_storage(StorageComponent *storage) { storage_ = storage; }
  void set_file_id(const std::string &file_id) { file_id_ = file_id; }
  void execute() override {  // Remplace play() par execute() si nécessaire
    storage_->play_file(file_id_);
  }

 private:
  StorageComponent *storage_;
  std::string file_id_;
};

class LoadImageAction : public esphome::core::Action {
 public:
  void set_storage(StorageComponent *storage) { storage_ = storage; }
  void set_image_id(const std::string &image_id) { image_id_ = image_id; }
  void execute() override {  // Remplace play() par execute() si nécessaire
    storage_->load_image(image_id_);
  }

 private:
  StorageComponent *storage_;
  std::string image_id_;
};

}  // namespace storage
}  // namespace esphome

