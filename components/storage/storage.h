#pragma once
#include "esphome.h"

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
        // Logique pour jouer le fichier audio
        break;
      }
    }
  }

  void load_image(const std::string &image_id) {
    for (const auto &image : images_) {
      if (image.second == image_id) {
        ESP_LOGD("storage", "Loading image: %s", image.first.c_str());
        // Logique pour accéder à l'image
        break;
      }
    }
  }

  void setup() override;  // Déclaration de la méthode sans la définir ici
  void setup_sd_card();   // Déclaration de la méthode sans la définir ici

 private:
  std::string platform_;
  std::vector<std::pair<std::string, std::string>> files_;
  std::vector<std::pair<std::string, std::string>> images_;

  void setup_flash();  // Définition des autres méthodes sans duplication
  void setup_inline();
};

}  // namespace storage
}  // namespace esphome










