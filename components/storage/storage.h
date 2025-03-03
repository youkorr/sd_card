#pragma once
#include "esphome.h"
#include <vector>
#include <fstream>

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

  std::vector<uint8_t> read_file(const char *path) {
    return read_file(std::string(path));
  }

  std::vector<uint8_t> read_file(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      ESP_LOGE("storage", "Failed to open file for reading: %s", path.c_str());
      return {};
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
      ESP_LOGE("storage", "Failed to read file: %s", path.c_str());
      return {};
    }
    return buffer;
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

// Déclaration des classes d'action compatibles avec ESPHome
template<typename... Ts>
class PlayAudioFileAction : public Action<Ts...> {
 public:
  PlayAudioFileAction() : storage_(nullptr) {}
  explicit PlayAudioFileAction(StorageComponent *storage) : storage_(storage) {}

  void set_storage(StorageComponent *storage) { storage_ = storage; }
  void set_file_id(const std::string &file_id) { file_id_ = file_id; }

  void play(Ts... x) override {
    if (storage_ != nullptr) {
      storage_->play_file(file_id_);
    } else {
      ESP_LOGE("storage", "Storage component not set for PlayAudioFileAction");
    }
  }

 protected:
  StorageComponent *storage_;
  std::string file_id_;
};

template<typename... Ts>
class LoadImageAction : public Action<Ts...> {
 public:
  LoadImageAction() : storage_(nullptr) {}
  explicit LoadImageAction(StorageComponent *storage) : storage_(storage) {}

  void set_storage(StorageComponent *storage) { storage_ = storage; }
  void set_image_id(const std::string &image_id) { image_id_ = image_id; }

  void play(Ts... x) override {
    if (storage_ != nullptr) {
      storage_->load_image(image_id_);
    } else {
      ESP_LOGE("storage", "Storage component not set for LoadImageAction");
    }
  }

 protected:
  StorageComponent *storage_;
  std::string image_id_;
};

}  // namespace storage
}  // namespace esphome








