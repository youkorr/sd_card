
#pragma once
#include "esphome.h"
#include "SD.h"

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
        if (platform_ == "sd") {
          play_media_from_sd(file.first);
        } else {
          // Logique existante pour les autres plateformes
        }
        break;
      }
    }
  }

  void load_image(const std::string &image_id) {
    for (const auto &image : images_) {
      if (image.second == image_id) {
        ESP_LOGD("storage", "Loading image: %s", image.first.c_str());
        if (platform_ == "sd") {
          load_image_from_sd(image.first);
        } else {
          // Logique existante pour les autres plateformes
        }
        break;
      }
    }
  }

  void setup() override {
    if (platform_ == "flash") {
      setup_flash();
    } else if (platform_ == "inline") {
      setup_inline();
    } else if (platform_ == "sd") {
      setup_sd();
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

  void setup_sd() {
    if (!SD.begin()) {
      ESP_LOGE("storage", "SD Card initialization failed!");
      return;
    }
    ESP_LOGI("storage", "SD Card initialized successfully");
    
    for (const auto &file : files_) {
      if (SD.exists(file.first.c_str())) {
        ESP_LOGD("storage", "File found on SD: %s", file.first.c_str());
      } else {
        ESP_LOGE("storage", "File not found on SD: %s", file.first.c_str());
      }
    }
  }

  void play_media_from_sd(const std::string &file_path) {
    if (SD.exists(file_path.c_str())) {
      File file = SD.open(file_path.c_str());
      if (file) {
        ESP_LOGI("storage", "Playing file from SD: %s", file_path.c_str());
        // Ajoutez ici la logique pour lire le fichier audio
        file.close();
      } else {
        ESP_LOGE("storage", "Failed to open file: %s", file_path.c_str());
      }
    } else {
      ESP_LOGE("storage", "File not found on SD: %s", file_path.c_str());
    }
  }

  void load_image_from_sd(const std::string &file_path) {
    if (SD.exists(file_path.c_str())) {
      File file = SD.open(file_path.c_str());
      if (file) {
        ESP_LOGI("storage", "Loading image from SD: %s", file_path.c_str());
        // Ajoutez ici la logique pour charger l'image
        file.close();
      } else {
        ESP_LOGE("storage", "Failed to open image: %s", file_path.c_str());
      }
    } else {
      ESP_LOGE("storage", "Image not found on SD







