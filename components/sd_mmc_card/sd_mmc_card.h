#pragma once
#include "esphome/core/gpio.h"
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#ifdef USE_ESP_IDF
#include "sdmmc_cmd.h"
#endif

namespace esphome {
namespace sd_mmc_card {

enum MemoryUnits : short { Byte = 0, KiloByte = 1, MegaByte = 2, GigaByte = 3, TeraByte = 4, PetaByte = 5 };

#ifdef USE_SENSOR
struct FileSizeSensor {
  sensor::Sensor *sensor{nullptr};
  std::string path;

  FileSizeSensor() = default;
  FileSizeSensor(sensor::Sensor *, std::string const &path);
};
#endif

struct FileInfo {
  std::string path;
  size_t size;
  bool is_directory;

  FileInfo(std::string const &, size_t, bool);
};

class SdMmc : public Component {
#ifdef USE_SENSOR
  SUB_SENSOR(used_space)
  SUB_SENSOR(total_space)
  SUB_SENSOR(free_space)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(sd_card_type)
#endif
 public:
  enum ErrorCode {
    ERR_PIN_SETUP,
    ERR_MOUNT,
    ERR_NO_CARD,
  };
  void setup() override;
  void loop() override;
  void dump_config() override;
  void write_file(const char *path, const uint8_t *buffer, size_t len, const char *mode);
  void write_file(const char *path, const uint8_t *buffer, size_t len);
  void append_file(const char *path, const uint8_t *buffer, size_t len);
  bool delete_file(const char *path);
  bool delete_file(std::string const &path);
  bool create_directory(const char *path);
  bool remove_directory(const char *path);

  // Nouveau code pour read_file
  std::vector<uint8_t> read_file(char const *path) {
    if (path == nullptr) {
      ESP_LOGE("sd_mmc", "Cannot read file: path is null");
      return {};
    }

    // Vérifier si le fichier existe
    if (!this->is_directory(path)) {
      FILE *file = fopen(path, "rb");
      if (file == nullptr) {
        ESP_LOGE("sd_mmc", "Failed to open file %s for reading", path);
        return {};
      }

      // Obtenir la taille du fichier
      fseek(file, 0, SEEK_END);
      size_t file_size = ftell(file);
      fseek(file, 0, SEEK_SET);

      // Créer un buffer de la taille du fichier
      std::vector<uint8_t> buffer(file_size);

      // Lire le contenu du fichier
      size_t bytes_read = fread(buffer.data(), 1, file_size, file);
      fclose(file);

      if (bytes_read != file_size) {
        ESP_LOGE("sd_mmc", "Failed to read file %s completely, expected %zu bytes but got %zu", 
                 path, file_size, bytes_read);
        buffer.resize(bytes_read);  // Ajuster la taille du buffer si nécessaire
      }

      ESP_LOGD("sd_mmc", "Successfully read %zu bytes from file %s", bytes_read, path);
      return buffer;
    } else {
      ESP_LOGE("sd_mmc", "Cannot read %s: it's a directory", path);
      return {};
    }
  }

  std::vector<uint8_t> read_file(std::string const &path) {
    return this->read_file(path.c_str());
  }

#ifdef USE_SENSOR
  void add_file_size_sensor(sensor::Sensor *, std::string const &path);
#endif

  void set_clk_pin(uint8_t);
  void set_cmd_pin(uint8_t);
  void set_data0_pin(uint8_t);
  void set_data1_pin(uint8_t);
  void set_data2_pin(uint8_t);
  void set_data3_pin(uint8_t);
  void set_mode_1bit(bool);
  void set_power_ctrl_pin(int8_t pin) { this->power_ctrl_pin_ = pin; }

 protected:
  ErrorCode init_error_;
  uint8_t clk_pin_;
  uint8_t cmd_pin_;
  uint8_t data0_pin_;
  uint8_t data1_pin_;
  uint8_t data2_pin_;
  uint8_t data3_pin_;
  bool mode_1bit_;
  int8_t power_ctrl_pin_{-1};  
#ifdef USE_ESP_IDF
  sdmmc_card_t *card_;
#endif
#ifdef USE_SENSOR
  std::vector<FileSizeSensor> file_size_sensors_{};
#endif
  void update_sensors();
#ifdef USE_ESP32_FRAMEWORK_ARDUINO
  std::string sd_card_type_to_string(int) const;
#endif
#ifdef USE_ESP_IDF
  std::string sd_card_type() const;
#endif
  std::vector<FileInfo> &list_directory_file_info_rec(const char *path, uint8_t depth, std::vector<FileInfo> &list);
  static std::string error_code_to_string(ErrorCode);
};

template<typename... Ts> class SdMmcWriteFileAction : public Action<Ts...> {
 public:
  SdMmcWriteFileAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)
  TEMPLATABLE_VALUE(std::vector<uint8_t>, data)

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    auto buffer = this->data_.value(x...);
    this->parent_->write_file(path.c_str(), buffer.data(), buffer.size());
  }

 protected:
  SdMmc *parent_;
};

template<typename... Ts> class SdMmcAppendFileAction : public Action<Ts...> {
 public:
  SdMmcAppendFileAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)
  TEMPLATABLE_VALUE(std::vector<uint8_t>, data)

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    auto buffer = this->data_.value(x...);
    this->parent_->append_file(path.c_str(), buffer.data(), buffer.size());
  }

 protected:
  SdMmc *parent_;
};

template<typename... Ts> class SdMmcCreateDirectoryAction : public Action<Ts...> {
 public:
  SdMmcCreateDirectoryAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    this->parent_->create_directory(path.c_str());
  }

 protected:
  SdMmc *parent_;
};

template<typename... Ts> class SdMmcRemoveDirectoryAction : public Action<Ts...> {
 public:
  SdMmcRemoveDirectoryAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    this->parent_->remove_directory(path.c_str());
  }

 protected:
  SdMmc *parent_;
};

template<typename... Ts> class SdMmcDeleteFileAction : public Action<Ts...> {
 public:
  SdMmcDeleteFileAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    this->parent_->delete_file(path.c_str());
  }

 protected:
  SdMmc *parent_;
};

long double convertBytes(uint64_t, MemoryUnits);

}  // namespace sd_mmc_card
}  // namespace esphome
