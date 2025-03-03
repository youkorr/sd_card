#pragma once
#include "esphome.h"
#include <FS.h>
#include <SD.h>

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
        read_file(file.first);  // Lire le fichier
        break;
      }
    }
  }

  void load_image(const std::string &image_id) {
    for (const auto &image : images_) {
      if (image.second == image_id) {
        ESP_LOGD("storage", "Loading image: %s", image.first.c_str());
        // Ajoutez ici la logique pour accéder à l'image
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

  void read_file(const std::string &file_path) {
    if (!SD.begin()) {
      ESP_LOGE("storage", "Failed to initialize SD card");
      return;
    }

    File file = SD.open(file_path.c_str(), FILE_READ);
    if (!file) {
      ESP_LOGE("storage", "Failed to open file: %s", file_path.c_str());
      return;
    }

    ESP_LOGD("storage", "Reading file: %s", file_path.c_str());
    while (file.available()) {
      // Lire et traiter les données du fichier
      uint8_t buffer[128];
      size_t bytes_read = file.read(buffer, sizeof(buffer));
      // Ajoutez ici la logique pour traiter les données audio
    }

    file.close();
  }
};

// Déclaration des classes d'action compatibles avec ESPHome
template<typename... Ts>
class PlayAudioFileAction : public Action<Ts...> {
 public:
  explicit PlayAudioFileAction(StorageComponent *storage) : storage_(storage) {}

  void set_file_id(const std::string &file_id) { file_id_ = file_id; }

  void play(Ts... x) override {
    storage_->play_file(file_id_);
  }

 protected:
  StorageComponent *storage_;
  std::string file_id_;
};

template<typename... Ts>
class LoadImageAction : public Action<Ts...> {
 public:
  explicit LoadImageAction(StorageComponent *storage) : storage_(storage) {}

  void set_image_id(const std::string &image_id) { image_id_ = image_id; }

  void play(Ts... x) override {
    storage_->load_image(image_id_);
  }

 protected:
  StorageComponent *storage_;
  std::string image_id_;
};

}  // namespace storage
}  // namespace esphome








