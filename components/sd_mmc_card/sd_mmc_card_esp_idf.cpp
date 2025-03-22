#include "sd_mmc_card.h"

#ifdef USE_ESP_IDF
#include "math.h"
#include "esphome/core/log.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_types.h"
#include <sys/stat.h> // Required for stat()

int constexpr SD_OCR_SDHC_CAP = (1 << 30);  // value defined in esp-idf/components/sdmmc/include/sd_protocol_defs.h

namespace esphome {
namespace sd_mmc_card {

static constexpr size_t FILE_PATH_MAX = ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN;
static const char *TAG = "sd_mmc_card";
static const std::string MOUNT_POINT("/sdcard");

std::string build_path(const char *path) { return MOUNT_POINT + path; }

void SdMmc::setup() {
  if (this->power_ctrl_pin_ != nullptr)
    this->power_ctrl_pin_->setup();

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

  if (this->mode_1bit_) {
    slot_config.width = 1;
  } else {
    slot_config.width = 4;
  }

#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slot_config.clk = static_cast<gpio_num_t>(this->clk_pin_);
  slot_config.cmd = static_cast<gpio_num_t>(this->cmd_pin_);
  slot_config.d0 = static_cast<gpio_num_t>(this->data0_pin_);

  if (!this->mode_1bit_) {
    slot_config.d1 = static_cast<gpio_num_t>(this->data1_pin_);
    slot_config.d2 = static_cast<gpio_num_t>(this->data2_pin_);
    slot_config.d3 = static_cast<gpio_num_t>(this->data3_pin_);
  }
#endif

  // Enable internal pullups on enabled pins. The internal pullups
  // are insufficient however, please make sure 10k external pullups are
  // connected on the bus. This is for debug / example purpose only.
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  auto ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT.c_str(), &host, &slot_config, &mount_config, &this->card_);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      this->init_error_ = ErrorCode::ERR_MOUNT;
    } else {
      this->init_error_ = ErrorCode::ERR_NO_CARD;
    }
    mark_failed();
    return;
  }

#ifdef USE_TEXT_SENSOR
  if (this->sd_card_type_text_sensor_ != nullptr)
    this->sd_card_type_text_sensor_->publish_state(sd_card_type());
#endif

  update_sensors();
}

bool SdMmc::write_file(const char *path, const uint8_t *buffer, size_t len, bool overwrite) {
  std::string absolut_path = build_path(path);
  const char *mode = overwrite ? "wb" : "ab"; // "wb" écrase, "ab" ajoute à la fin
  FILE *file = fopen(absolut_path.c_str(), mode);
  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return false;
  }
  size_t written = fwrite(buffer, 1, len, file);
  fclose(file);
  this->update_sensors();
  return written == len; // Retourne true si tous les octets ont été écrits
}

void SdMmc::append_file(const char *path, const uint8_t *buffer, size_t len) {
  std::string absolut_path = build_path(path);
  FILE *file = fopen(absolut_path.c_str(), "ab");
  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for appending");
    return;
  }
  fwrite(buffer, 1, len, file);
  fclose(file);
  this->update_sensors();
}

bool SdMmc::create_directory(const char *path) {
  ESP_LOGV(TAG, "Create directory: %s", path);
  std::string absolut_path = build_path(path);
  if (mkdir(absolut_path.c_str(), 0777) < 0) {
    ESP_LOGE(TAG, "Failed to create a new directory: %s", strerror(errno));
    return false;
  }
  this->update_sensors();
  return true;
}

bool SdMmc::remove_directory(const char *path) {
  ESP_LOGV(TAG, "Remove directory: %s", path);
  if (!this->is_directory(path)) {
    ESP_LOGE(TAG, "Not a directory");
    return false;
  }
  std::string absolut_path = build_path(path);
  if (rmdir(absolut_path.c_str()) != 0) { // Use rmdir for directories
    ESP_LOGE(TAG, "Failed to remove directory: %s", strerror(errno));
    return false;
  }
  this->update_sensors();
  return true;
}

bool SdMmc::delete_file(const char *path) {
  ESP_LOGV(TAG, "Delete File: %s", path);
  if (this->is_directory(path)) {
    ESP_LOGE(TAG, "Not a file");
    return false;
  }
  std::string absolut_path = build_path(path);
  if (remove(absolut_path.c_str()) != 0) {
    ESP_LOGE(TAG, "Failed to remove file: %s", strerror(errno));
    return false;
  }
  this->update_sensors();
  return true;
}

std::vector<uint8_t> SdMmc::read_file(char const *path) {
  ESP_LOGV(TAG, "Read File: %s", path);

  std::string absolut_path = build_path(path);
  FILE *file = nullptr;
  file = fopen(absolut_path.c_str(), "rb");
  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return std::vector<uint8_t>();
  }

  std::vector<uint8_t> res;
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  res.resize(fileSize);
  size_t len = fread(res.data(), 1, fileSize, file);
  fclose(file);
  if (len != fileSize) {
    ESP_LOGE(TAG, "Failed to read file: %s", strerror(errno));
    return std::vector<uint8_t>();
  }

  return res;
}

size_t SdMmc::read_file_block(const std::string& path, uint8_t* buffer, size_t maxLen, size_t index) {
  std::string absolut_path = build_path(path.c_str());
  FILE *file = fopen(absolut_path.c_str(), "rb");
  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for reading: %s", absolut_path.c_str());
    return 0;
  }
  fseek(file, index, SEEK_SET);
  size_t bytesRead = fread(buffer, 1, maxLen, file);
  fclose(file);
  return bytesRead;
}

size_t SdMmc::file_size(const std::string &path) const {
  std::string absolut_path = build_path(path.c_str());
  struct stat st;
  if (stat(absolut_path.c_str(), &st) == 0) {
    return st.st_size;
  }
  ESP_LOGW(TAG, "Failed to get file size for %s", absolut_path.c_str());
  return 0;
}

std::vector<FileInfo> &SdMmc::list_directory_file_info_rec(const char *path, uint8_t depth, std::vector<FileInfo> &list) {
  std::string absolut_path = build_path(path);
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(absolut_path.c_str())) != nullptr) {
    while ((ent = readdir(dir)) != nullptr) {
      if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
        continue;
      }

      std::string full_path = absolut_path + "/" + ent->d_name;
      struct stat st;
      if (stat(full_path.c_str(), &st) == 0) {
        bool is_dir = (ent->d_type == DT_DIR);
        list.emplace_back(full_path, st.st_size, is_dir);
      } else {
        ESP_LOGW(TAG, "Error getting file information for %s", full_path.c_str());
      }
    }
    closedir(dir);
  } else {
    ESP_LOGE(TAG, "Could not open directory");
  }
  return list;
}

bool SdMmc::is_directory(const char *path) {
  std::string absolut_path = build_path(path);
  struct stat st;
  if (stat(absolut_path.c_str(), &st) == 0) {
    return S_ISDIR(st.st_mode);
  }
  return false;
}

}  // namespace sd_mmc_card
}  // namespace esphome

#endif

