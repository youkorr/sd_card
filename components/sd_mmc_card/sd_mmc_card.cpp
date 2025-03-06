#include "sd_mmc_card.h"
#include "esphome/core/log.h"
#include <sys/stat.h>
#include <algorithm>
#include <cmath>

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_card";

#ifdef USE_SENSOR
FileSizeSensor::FileSizeSensor(sensor::Sensor *sensor, std::string const &path) : sensor(sensor), path(path) {}
#endif

void SdMmc::loop() {}

void SdMmc::dump_config() {
  ESP_LOGCONFIG(TAG, "SD MMC Component");
  ESP_LOGCONFIG(TAG, "  Mode 1 bit: %s", TRUEFALSE(this->mode_1bit_));
  ESP_LOGCONFIG(TAG, "  CLK Pin: %d", this->clk_pin_);
  ESP_LOGCONFIG(TAG, "  CMD Pin: %d", this->cmd_pin_);
  ESP_LOGCONFIG(TAG, "  DATA0 Pin: %d", this->data0_pin_);
  if (!this->mode_1bit_) {
    ESP_LOGCONFIG(TAG, "  DATA1 Pin: %d", this->data1_pin_);
    ESP_LOGCONFIG(TAG, "  DATA2 Pin: %d", this->data2_pin_);
    ESP_LOGCONFIG(TAG, "  DATA3 Pin: %d", this->data3_pin_);
  }
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "Used space", this->used_space_sensor_);
  LOG_SENSOR("  ", "Total space", this->total_space_sensor_);
  LOG_SENSOR("  ", "Free space", this->free_space_sensor_);
  for (auto &sensor : this->file_size_sensors_) {
    if (sensor.sensor != nullptr)
      LOG_SENSOR("  ", "File size", sensor.sensor);
  }
#endif
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "SD Card Type", this->sd_card_type_text_sensor_);
#endif

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Setup failed : %s", SdMmc::error_code_to_string(this->init_error_).c_str());
    return;
  }
}

void SdMmc::write_file(const char *path, const uint8_t *buffer, size_t len) {
  ESP_LOGV(TAG, "Writing to file: %s", path);
  this->write_file(path, buffer, len, "w");
}

void SdMmc::append_file(const char *path, const uint8_t *buffer, size_t len) {
  ESP_LOGV(TAG, "Appending to file: %s", path);
  this->write_file(path, buffer, len, "a");
}

std::vector<std::string> SdMmc::list_directory(const char *path, uint8_t depth) {
  std::vector<std::string> list;
  std::vector<FileInfo> infos = list_directory_file_info(path, depth);
  list.resize(infos.size()); // Assurez-vous que la liste a la bonne taille
  std::transform(infos.cbegin(), infos.cend(), list.begin(), [](FileInfo const &info) { return info.path; });
  return list;
}

std::vector<std::string> SdMmc::list_directory(std::string path, uint8_t depth) {
  return this->list_directory(path.c_str(), depth);
}

std::vector<FileInfo> SdMmc::list_directory_file_info(const char *path, uint8_t depth) {
  std::vector<FileInfo> list;
  list_directory_file_info_rec(path, depth, list);
  return list;
}

std::vector<FileInfo> SdMmc::list_directory_file_info(std::string path, uint8_t depth) {
  return this->list_directory_file_info(path.c_str(), depth);
}

size_t SdMmc::file_size(const char *path) {
  if (path == nullptr) {
    ESP_LOGE(TAG, "Path is null");
    return 0;
  }
  struct stat file_stat;
  if (stat(path, &file_stat) != 0) {
    ESP_LOGW(TAG, "File does not exist: %s", path);
    return 0;
  }
  return file_stat.st_size;
}

size_t SdMmc::file_size(std::string const &path) {
  return this->file_size(path.c_str());
}

bool SdMmc::is_directory(const char *path) {
  if (path == nullptr) {
    ESP_LOGE(TAG, "Path is null");
    return false;
  }
  struct stat path_stat;
  if (stat(path, &path_stat) != 0) {
    ESP_LOGW(TAG, "Path does not exist: %s", path);
    return false;
  }
  return S_ISDIR(path_stat.st_mode);
}

bool SdMmc::is_directory(std::string const &path) {
  return this->is_directory(path.c_str());
}

bool SdMmc::delete_file(const char *path) {
    if (path == nullptr) {
    ESP_LOGE(TAG, "Path is null");
    return false;
  }
  if (remove(path) == 0) {
    ESP_LOGD(TAG, "File %s deleted successfully", path);
    return true;
  } else {
    ESP_LOGE(TAG, "Unable to delete file %s", path);
    return false;
  }
}


bool SdMmc::delete_file(std::string const &path) { return this->delete_file(path.c_str()); }

std::vector<uint8_t> SdMmc::read_file(const char *path) {
  if (path == nullptr) {
    ESP_LOGE(TAG, "Cannot read file: path is null");
    return {};
  }

  if (!this->is_directory(path)) {
    FILE *file = fopen(path, "rb");
    if (file == nullptr) {
      ESP_LOGE(TAG, "Failed to open file %s for reading", path);
      return {};
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::vector<uint8_t> buffer(file_size);
    size_t bytes_read = fread(buffer.data(), 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
      ESP_LOGE(TAG, "Failed to read file %s completely, expected %zu bytes but got %zu",
               path, file_size, bytes_read);
      buffer.resize(bytes_read);
    }

    ESP_LOGD(TAG, "Successfully read %zu bytes from file %s", bytes_read, path);
    return buffer;
  } else {
    ESP_LOGE(TAG, "Cannot read %s: it's a directory", path);
    return {};
  }
}


std::vector<uint8_t> SdMmc::read_file(std::string const &path) { return this->read_file(path.c_str()); }

#ifdef USE_SENSOR
void SdMmc::add_file_size_sensor(sensor::Sensor *sensor, std::string const &path) {
  this->file_size_sensors_.emplace_back(sensor, path);
}
#endif

void SdMmc::set_clk_pin(uint8_t pin) { this->clk_pin_ = pin; }

void SdMmc::set_cmd_pin(uint8_t pin) { this->cmd_pin_ = pin; }

void SdMmc::set_data0_pin(uint8_t pin) { this->data0_pin_ = pin; }

void SdMmc::set_data1_pin(uint8_t pin) { this->data1_pin_ = pin; }

void SdMmc::set_data2_pin(uint8_t pin) { this->data2_pin_ = pin; }

void SdMmc::set_data3_pin(uint8_t pin) { this->data3_pin_ = pin; }

void SdMmc::set_mode_1bit(bool b) { this->mode_1bit_ = b; }

std::string SdMmc::error_code_to_string(SdMmc::ErrorCode code) {
  switch (code) {
    case ErrorCode::ERR_PIN_SETUP:
      return "Failed to set pins";
    case ErrorCode::ERR_MOUNT:
      return "Failed to mount card";
    case ErrorCode::ERR_NO_CARD:
      return "No card found";
    default:
      return "Unknown error";
  }
}

std::vector<FileInfo> &SdMmc::list_directory_file_info_rec(const char *path, uint8_t depth, std::vector<FileInfo> &list) {
    // Implémentez la logique récursive ici pour lister les fichiers et dossiers
    // et ajoutez les FileInfo à la liste.
    return list;
}

long double convertBytes(uint64_t value, MemoryUnits unit) {
  return value * 1.0 / pow(1024, static_cast<uint64_t>(unit));
}

FileInfo::FileInfo(std::string const &path, size_t size, bool is_directory)
    : path(path), size(size), is_directory(is_directory) {}

}  // namespace sd_mmc_card
}  // namespace esphome
