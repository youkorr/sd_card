#include "storage.h"
#include "esphome/core/log.h"
#include "esphome/components/media_player/media_player.h"

namespace esphome {
namespace storage {

static const char *const TAG = "storage";

void StorageComponent::play_media(const std::string &media_file) {
  ESP_LOGD(TAG, "Playing media file: %s", media_file.c_str());

  // Construire le chemin de base du fichier sur la carte SD
  std::string base_path = "/sd/" + media_file;

  // Liste des extensions supportées
  std::vector<std::string> supported_extensions = {".mp3", ".flac"};

  // Vérifier l'existence du fichier (simulation)
  bool file_exists = false;
  std::string found_file_path;

  for (const auto &ext : supported_extensions) {
    std::string full_path = base_path + ext;
    // Ici, vous devrez implémenter une vérification réelle de l'existence du fichier
    // Par exemple, en utilisant une bibliothèque SD spécifique à votre plateforme
    ESP_LOGD(TAG, "Checking file: %s", full_path.c_str());
    
    // Simulation d'existence de fichier
    if (full_path == "/sd/timer_sound.mp3") {
      file_exists = true;
      found_file_path = full_path;
      break;
    }
  }

  if (file_exists) {
    // Trouver le média player
    auto *media_player = media_player::MediaPlayer::find_by_name("box3");
    
    if (media_player) {
      // Définir l'URL du fichier sur la carte SD
      media_player->set_media_url(found_file_path);
      
      // Jouer le média
      media_player->play();
      
      ESP_LOGD(TAG, "Playing media from SD card: %s", found_file_path.c_str());
    } else {
      ESP_LOGE(TAG, "Media player 'box3' not found");
    }
  } else {
    ESP_LOGE(TAG, "Media file not found on SD card: %s", base_path.c_str());
  }
}

void StorageComponent::setup() {
  ESP_LOGD(TAG, "Setting up storage component");

  if (platform_ == "flash") {
    setup_flash();
  } else if (platform_ == "inline") {
    setup_inline();
  } else if (platform_ == "sd_card") {
    setup_sd_card();
  }
}

void StorageComponent::setup_sd_card() {
  ESP_LOGD(TAG, "Setting up SD card storage...");
  
  // Simulation de listage de fichiers
  std::vector<std::string> files = {
    "timer_sound.mp3",
    "timer_sound.flac",
    "alarm_sound.mp3"
  };

  for (const auto &file : files) {
    ESP_LOGD(TAG, "Found file on SD card: %s", file.c_str());
  }
}

void StorageComponent::setup_flash() {
  for (const auto &file : files_) {
    ESP_LOGD(TAG, "Setting up flash storage: %s -> %s", 
             file.first.c_str(), file.second.c_str());
  }
}

void StorageComponent::setup_inline() {
  for (const auto &file : files_) {
    ESP_LOGD(TAG, "Setting up inline storage: %s -> %s", 
             file.first.c_str(), file.second.c_str());
  }
}

void StorageComponent::load_image(const std::string &image_id) {
  ESP_LOGD(TAG, "Loading image: %s", image_id.c_str());
}

}  // namespace storage
}  // namespace esphome



