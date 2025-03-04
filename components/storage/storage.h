#pragma once
#include "esphome.h"
#include "esp32/SD_MMC.h"  // Utiliser SD_MMC pour ESP32-S3

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

  void play_media(const std::string &media_file) {
    for (const auto &file : files_) {
      if (file.second == media_file) {
        ESP_LOGD("storage", "Playing media: %s", file.first.c_str());
        // Ajoutez ici la logique pour jouer le fichier audio
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
    } else if (platform_ == "sd_card") {
      setup_sd_card();  // Appel de la méthode pour initialiser la carte SD
    }
  }

 private:
  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
  std::vector<std::pair<std::string, std::string>> images_;

  // Nouvelle méthode pour gérer l'initialisation de la carte SD
  void setup_sd_card() {
    if (!SD_MMC.begin("/sdcard", true)) {  // "/sdcard" est le chemin de montage de la carte SD
      ESP_LOGE("storage", "Card failed, or not present");
      return;  // Si la carte SD ne peut pas être initialisée, on retourne immédiatement
    }
    ESP_LOGD("storage", "Card mounted successfully");
  }

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








