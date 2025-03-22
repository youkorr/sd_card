#include "sd_mmc_card.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sd_mmc_card {

static const char *TAG = "sd_mmc_file_stream";

FileStream::~FileStream() {
  this->close();
}

bool FileStream::open_read(const char* path) {
  this->close();
  this->file_ = fopen(path, "rb");
  if (this->file_ == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
    return false;
  }
  
  // Déterminer la taille du fichier
  fseek(this->file_, 0, SEEK_END);
  this->file_size_ = ftell(this->file_);
  fseek(this->file_, 0, SEEK_SET);
  
  ESP_LOGV(TAG, "Opened file for reading: %s (size: %s)", path, format_size(this->file_size_).c_str());
  return true;
}

bool FileStream::open_write(const char* path, const char* mode) {
  this->close();
  this->file_ = fopen(path, mode);
  if (this->file_ == nullptr) {
    ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
    return false;
  }
  
  ESP_LOGV(TAG, "Opened file for writing: %s", path);
  return true;
}

size_t FileStream::read(uint8_t* buffer, size_t max_size) {
  if (!this->is_open()) {
    ESP_LOGE(TAG, "Attempted to read from closed file");
    return 0;
  }
  
  size_t bytes_read = fread(buffer, 1, max_size, this->file_);
  if (bytes_read < max_size && !feof(this->file_)) {
    ESP_LOGE(TAG, "Error reading from file");
  }
  
  return bytes_read;
}

size_t FileStream::write(const uint8_t* buffer, size_t len) {
  if (!this->is_open()) {
    ESP_LOGE(TAG, "Attempted to write to closed file");
    return 0;
  }
  
  size_t bytes_written = fwrite(buffer, 1, len, this->file_);
  if (bytes_written < len) {
    ESP_LOGE(TAG, "Error writing to file");
  }
  
  return bytes_written;
}

bool FileStream::eof() const {
  if (!this->is_open()) 
    return true;
    
  return feof(this->file_) != 0;
}

void FileStream::close() {
  if (this->file_ != nullptr) {
    fclose(this->file_);
    this->file_ = nullptr;
    this->file_size_ = 0;
  }
}

bool FileStream::is_open() const {
  return this->file_ != nullptr;
}

size_t FileStream::size() const {
  return this->file_size_;
}

size_t FileStream::tell() const {
  if (!this->is_open())
    return 0;
    
  return ftell(this->file_);
}

bool FileStream::seek(size_t position) {
  if (!this->is_open())
    return false;
    
  return fseek(this->file_, position, SEEK_SET) == 0;
}

}  // namespace sd_mmc_card
}  // namespace esphome
