#include "storage.h"

namespace esphome {
namespace storage {

static const char *const TAG = "storage";

void StorageComponent::setup() {
  if (platform_ == "flash") {
    setup_flash();
  } else if (platform_ == "inline") {
    setup_inline();
  } else if (platform_ == "sd_card") {
    setup_sd_card();  // Appel à la méthode de gestion de la carte SD
  }
}

void StorageComponent::setup_sd_card() {
  ESP_LOGD("storage", "Setting up SD card...");
  // Pas besoin d'appeler SD_MMC.begin() ici. ESPHome gère cela avec la configuration YAML.
}

void StorageComponent::setup_flash() {
  for (const auto &file : files_) {
    ESP_LOGD("storage", "Setting up flash storage: %s -> %s", 
             file.first.c_str(), file.second.c_str());
  }
}

void StorageComponent::setup_inline() {
  for (const auto &file : files_) {
    ESP_LOGD("storage", "Setting up inline storage: %s -> %s", 
             file.first.c_str(), file.second.c_str());
  }
}

}  // namespace storage
}  // namespace esphome



