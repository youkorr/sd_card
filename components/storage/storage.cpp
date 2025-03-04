#include "storage.h"
#include "esphome/core/log.h"
#include "esphome/components/media_player/media_player.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include <sys/stat.h>
#include <dirent.h>

namespace esphome {
namespace storage {

static const char *const TAG = "storage";

bool StorageComponent::file_exists(const std::string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

void StorageComponent::play_media(const std::string &media_file) {
    ESP_LOGD(TAG, "Playing media file: %s", media_file.c_str());

    std::string base_path = "/sd/" + media_file;
    std::vector<std::string> supported_extensions = {".mp3", ".flac"};
    bool file_found = false;
    std::string found_file_path;

    for (const auto &ext : supported_extensions) {
        std::string full_path = base_path + ext;
        ESP_LOGD(TAG, "Checking file: %s", full_path.c_str());
        
        if (file_exists(full_path)) {
            file_found = true;
            found_file_path = full_path;
            break;
        }
    }

    if (file_found) {
        auto *media_player = esphome::media_player::MediaPlayer::get_component("box3");
        if (media_player) {
            media_player->set_media_url(found_file_path);
            media_player->play();
            ESP_LOGD(TAG, "Playing media from SD card: %s", found_file_path.c_str());
        } else {
            ESP_LOGE(TAG, "Media player 'box3' not found");
        }
    } else {
        ESP_LOGE(TAG, "Media file not found on SD card: %s", base_path.c_str());
    }
}

void StorageComponent::setup_sd_card() {
    ESP_LOGD(TAG, "Setting up SD card storage...");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sd", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Échec du montage de la carte SD. Vérifiez le formatage (FAT32).");
        } else {
            ESP_LOGE(TAG, "Échec de l'initialisation de la carte SD (erreur : %s)", esp_err_to_name(ret));
        }
        return;
    }

    ESP_LOGD(TAG, "Carte SD initialisée avec succès !");
    list_files("/sd");
}

void StorageComponent::list_files(const std::string &path) {
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        ESP_LOGE(TAG, "Impossible d'ouvrir le répertoire : %s", path.c_str());
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        ESP_LOGD(TAG, "Fichier trouvé : %s", entry->d_name);
    }

    closedir(dir);
}

// Autres méthodes...
}  // namespace storage
}  // namespace esphome



