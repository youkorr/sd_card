#include "sd_mmc_card.h"

#include <algorithm>
#include <memory>
#include <dirent.h>       // Pour opendir, readdir, closedir
#include <sys/stat.h>     // Pour stat, S_ISDIR
#include <unistd.h>       // Pour unlink, rmdir

#include "math.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_card";

bool SdMmc::exists(const std::string &path) {
  FILE *file = fopen(path.c_str(), "rb");
  if (file != nullptr) {
    fclose(file);
    return true;
  }
  return false;
}

size_t SdMmc::get_file_size(const std::string &path) {
  FILE *file = fopen(path.c_str(), "rb");
  if (file == nullptr) {
    return 0;
  }
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fclose(file);
  return size;
}

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

  if (this->power_ctrl_pin_ != nullptr) {
    LOG_PIN("  Power Ctrl Pin: ", this->power_ctrl_pin_);
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

void SdMmc::write_file(const char *path, const uint8_t *buffer, size_t len, const char *mode) {
  auto stream = this->open_file_write(path, mode);
  if (stream && stream->is_open()) {
    stream->write(buffer, len);
  }
}

void SdMmc::append_file(const char *path, const uint8_t *buffer, size_t len) {
  ESP_LOGV(TAG, "Appending to file: %s", path);
  this->write_file(path, buffer, len, "a");
}

std::vector<uint8_t> SdMmc::read_file(const char *path) {
  std::vector<uint8_t> content;
  size_t file_size = this->get_file_size(path);
  
  if (file_size == 0) {
    ESP_LOGE(TAG, "Cannot read file (empty or not found): %s", path);
    return content;
  }
  
  // Pour compatibilité, utilisons encore la méthode traditionnelle
  // Tout en gardant la possibilité d'utiliser le streaming pour les gros fichiers
  if (file_size > 1024 * 1024) {  // Plus de 1MB
    ESP_LOGW(TAG, "Reading large file (%s), consider using streaming APIs instead", format_size(file_size).c_str());
  }
  
  content.resize(file_size);
  auto stream = this->open_file_read(path);
  if (stream && stream->is_open()) {
    size_t bytes_read = stream->read(content.data(), file_size);
    if (bytes_read != file_size) {
      content.resize(bytes_read);
      ESP_LOGW(TAG, "Read fewer bytes than expected (%d vs %d)", bytes_read, file_size);
    }
  } else {
    ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
    content.clear();
  }
  
  return content;
}

std::vector<uint8_t> SdMmc::read_file(std::string const &path) {
  return this->read_file(path.c_str());
}

std::unique_ptr<FileStream> SdMmc::open_file_read(const char* path) {
  std::unique_ptr<FileStream> stream = std::make_unique<FileStream>();
  if (!stream->open_read(path)) {
    ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
    return nullptr;
  }
  return stream;
}

std::unique_ptr<FileStream> SdMmc::open_file_read(const std::string& path) {
  return this->open_file_read(path.c_str());
}

std::unique_ptr<FileStream> SdMmc::open_file_write(const char* path, const char* mode) {
  std::unique_ptr<FileStream> stream = std::make_unique<FileStream>();
  if (!stream->open_write(path, mode)) {
    ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
    return nullptr;
  }
  return stream;
}

std::unique_ptr<FileStream> SdMmc::open_file_write(const std::string& path, const char* mode) {
  return this->open_file_write(path.c_str(), mode);
}

bool SdMmc::process_file(const char* path, ReadCallback callback, size_t buffer_size) {
  size_t file_size = this->get_file_size(path);
  if (file_size == 0) {
    ESP_LOGW(TAG, "File empty or not found: %s", path);
    return false;
  }
  
  auto stream = this->open_file_read(path);
  if (!stream || !stream->is_open()) {
    ESP_LOGE(TAG, "Failed to open file for processing: %s", path);
    return false;
  }
  
  std::vector<uint8_t> buffer(buffer_size);
  size_t bytes_read = 0;
  size_t total_read = 0;
  bool result = true;
  
  ESP_LOGD(TAG, "Processing file: %s with buffer size: %s", path, format_size(buffer_size).c_str());
  
  while ((bytes_read = stream->read(buffer.data(), buffer_size)) > 0) {
    result = callback(buffer.data(), bytes_read, file_size, total_read);
    total_read += bytes_read;
    
    if (!result) {
      ESP_LOGW(TAG, "File processing callback requested early termination");
      break;
    }
  }
  
  if (total_read != file_size && result) {
    ESP_LOGW(TAG, "File processing incomplete: %d/%d bytes processed", total_read, file_size);
    result = false;
  }
  
  return result;
}

bool SdMmc::process_file(const std::string& path, ReadCallback callback, size_t buffer_size) {
  return this->process_file(path.c_str(), callback, buffer_size);
}

bool SdMmc::write_file_stream(const char* path, WriteCallback callback, size_t buffer_size) {
  auto stream = this->open_file_write(path);
  if (!stream || !stream->is_open()) {
    ESP_LOGE(TAG, "Failed to open file for streaming write: %s", path);
    return false;
  }
  
  std::vector<uint8_t> buffer(buffer_size);
  size_t bytes_to_write = 0;
  size_t total_written = 0;
  
  ESP_LOGD(TAG, "Writing file in stream mode: %s with buffer size: %s", path, format_size(buffer_size).c_str());
  
  while ((bytes_to_write = callback(buffer.data(), buffer_size)) > 0) {
    size_t bytes_written = stream->write(buffer.data(), bytes_to_write);
    if (bytes_written != bytes_to_write) {
      ESP_LOGE(TAG, "Write error: wrote %d/%d bytes", bytes_written, bytes_to_write);
      return false;
    }
    total_written += bytes_written;
    
    // Si le callback renvoie moins que la taille du buffer, c'est la fin du flux
    if (bytes_to_write < buffer_size) {
      break;
    }
  }
  
  ESP_LOGD(TAG, "Finished writing file: %s, total bytes: %s", path, format_size(total_written).c_str());
  return true;
}

bool SdMmc::write_file_stream(const std::string& path, WriteCallback callback, size_t buffer_size) {
  return this->write_file_stream(path.c_str(), callback, buffer_size);
}

std::vector<std::string> SdMmc::list_directory(const char *path, uint8_t depth) {
  std::vector<std::string> list;
  std::vector<FileInfo> infos = list_directory_file_info(path, depth);
  list.resize(infos.size());
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

void SdMmc::list_directory_file_info_rec(const char *path, uint8_t depth, std::vector<FileInfo> &file_info) {
  DIR *dir = opendir(path);
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open directory: %s", path);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string full_path = std::string(path) + "/" + entry->d_name;

    // Ignore les entrées spéciales "." et ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Vérifie si c'est un répertoire
    bool is_dir = (entry->d_type == DT_DIR);

    // Ajoute les informations du fichier/dossier à la liste
    file_info.emplace_back(full_path, get_file_size(full_path), is_dir);

    // Si c'est un répertoire et que la profondeur n'est pas atteinte, parcourt récursivement
    if (is_dir && depth > 0) {
      list_directory_file_info_rec(full_path.c_str(), depth - 1, file_info);
    }
  }

  closedir(dir);
}

bool SdMmc::is_directory(const char *path) {
  struct stat path_stat;
  if (stat(path, &path_stat) != 0) {
    ESP_LOGE(TAG, "Failed to stat path: %s", path);
    return false;
  }
  return S_ISDIR(path_stat.st_mode);
}

bool SdMmc::delete_file(const char *path) {
  if (is_directory(path)) {
    // Supprime un répertoire récursivement
    std::vector<FileInfo> files = list_directory_file_info(path, 0);
    for (const auto &file : files) {
      if (!delete_file(file.path.c_str())) {
        return false;
      }
    }
    if (rmdir(path) != 0) {
      ESP_LOGE(TAG, "Failed to delete directory: %s", path);
      return false;
    }
  } else {
    // Supprime un fichier
    if (unlink(path) != 0) {
      ESP_LOGE(TAG, "Failed to delete file: %s", path);
      return false;
    }
  }
  return true;
}

size_t SdMmc::file_size(std::string const &path) { return this->file_size(path.c_str()); }

bool SdMmc::is_directory(std::string const &path) { return this->is_directory(path.c_str()); }

bool SdMmc::delete_file(std::string const &path) { return this->delete_file(path.c_str()); }

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

void SdMmc::set_power_ctrl_pin(GPIOPin *pin) { this->power_ctrl_pin_ = pin; }

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

long double convertBytes(uint64_t value, MemoryUnits unit) {
  return value * 1.0 / pow(1024, static_cast<uint64_t>(unit));
}

std::string memory_unit_to_string(MemoryUnits unit) {
  switch (unit) {
    case MemoryUnits::Byte:
      return "B";
    case MemoryUnits::KiloByte:
      return "KB";
    case MemoryUnits::MegaByte:
      return "MB";
    case MemoryUnits::GigaByte:
      return "GB";
    case MemoryUnits::TeraByte:
      return "TB";
    case MemoryUnits::PetaByte:
      return "PB";
  }
  return "unknown";
}

MemoryUnits memory_unit_from_size(size_t size) {
  short unit = MemoryUnits::Byte;
  double s = static_cast<double>(size);
  while (s >= 1024 && unit < MemoryUnits::PetaByte) {
    s /= 1024;
    unit++;
  }
  return static_cast<MemoryUnits>(unit);
}

std::string format_size(size_t size) {
  MemoryUnits unit = memory_unit_from_size(size);
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%.2f %s", convertBytes(size, unit), memory_unit_to_string(unit).c_str());
  return std::string(buffer);
}

FileInfo::FileInfo(std::string const &path, size_t size, bool is_directory)
    : path(path), size(size), is_directory(is_directory) {}

}  // namespace sd_mmc_card
}  // namespace esphome
