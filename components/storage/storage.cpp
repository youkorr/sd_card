#include "storage.h"


namespace esphome {
namespace storage {

static const char *const TAG = "storage";

// Ajout des méthodes setup() et setup_sd_card()
void StorageComponent::setup() {
  if (platform_ == "flash") {
    setup_flash();
  } else if (platform_ == "inline") {
    setup_inline();
  } else if (platform_ == "sd_card") {
    setup_sd_card();  // Appel de la méthode pour initialiser la carte SD
  }
}

void StorageComponent::setup_sd_card() {
  // Initialise la carte SD via SD_MMC
  if (!SD_MMC.begin("/sdcard", true)) {  // "/sdcard" est le chemin de montage de la carte SD
    ESP_LOGE(TAG, "Card failed, or not present");
    return;  // Si la carte SD ne peut pas être initialisée, on retourne immédiatement
  }
  ESP_LOGD(TAG, "Card mounted successfully");
}

}  // namespace storage
}  // namespace esphome


