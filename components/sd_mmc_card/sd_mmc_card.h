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

// Taille du buffer pour le streaming
static constexpr size_t DEFAULT_STREAM_BUFFER_SIZE = 1024;

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

// Classe pour les opérations de streaming sur les fichiers
class FileStream {
 public:
  FileStream() = default;
  ~FileStream();
  
  // Ouvre un fichier en mode lecture
  bool open_read(const char* path);
  
  // Ouvre un fichier en mode écriture
  bool open_write(const char* path, const char* mode);
  
  // Lit un bloc de données
  size_t read(uint8_t* buffer, size_t max_size);
  
  // Écrit un bloc de données
  size_t write(const uint8_t* buffer, size_t len);
  
  // Renvoie si le stream est arrivé à la fin
  bool eof() const;
  
  // Ferme le fichier
  void close();
  
  // Renvoie si le fichier est ouvert
  bool is_open() const;
  
  // Obtient la taille du fichier
  size_t size() const;
  
  // Position actuelle dans le fichier
  size_t tell() const;
  
  // Déplace la position dans le fichier
  bool seek(size_t position);

 private:
  FILE* file_{nullptr};
  size_t file_size_{0};
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
  
  // Méthodes de fichier traditionnelles
  void write_file(const char *path, const uint8_t *buffer, size_t len, const char *mode);
  void write_file(const char *path, const uint8_t *buffer, size_t len);
  void append_file(const char *path, const uint8_t *buffer, size_t len);
  bool delete_file(const char *path);
  bool delete_file(std::string const &path);
  bool create_directory(const char *path);
  bool remove_directory(const char *path);
  bool exists(const std::string &path);
  size_t get_file_size(const std::string &path);
  std::vector<uint8_t> read_file(char const *path);
  std::vector<uint8_t> read_file(std::string const &path);
  
  // Nouvelles méthodes pour le streaming
  std::unique_ptr<FileStream> open_file_read(const char* path);
  std::unique_ptr<FileStream> open_file_read(const std::string& path);
  std::unique_ptr<FileStream> open_file_write(const char* path, const char* mode = "w");
  std::unique_ptr<FileStream> open_file_write(const std::string& path, const char* mode = "w");
  
  // Callbacks pour le traitement de fichier par morceaux
  using ReadCallback = std::function<bool(const uint8_t* data, size_t size, size_t total_size, size_t position)>;
  using WriteCallback = std::function<size_t(uint8_t* buffer, size_t max_size)>;
  
  // Traitement d'un fichier par streaming avec callbacks
  bool process_file(const char* path, ReadCallback callback, size_t buffer_size = DEFAULT_STREAM_BUFFER_SIZE);
  bool process_file(const std::string& path, ReadCallback callback, size_t buffer_size = DEFAULT_STREAM_BUFFER_SIZE);
  bool write_file_stream(const char* path, WriteCallback callback, size_t buffer_size = DEFAULT_STREAM_BUFFER_SIZE);
  bool write_file_stream(const std::string& path, WriteCallback callback, size_t buffer_size = DEFAULT_STREAM_BUFFER_SIZE);

  bool is_directory(const char *path);
  bool is_directory(std::string const &path);
  std::vector<std::string> list_directory(const char *path, uint8_t depth);
  std::vector<std::string> list_directory(std::string path, uint8_t depth);
  std::vector<FileInfo> list_directory_file_info(const char *path, uint8_t depth);
  std::vector<FileInfo> list_directory_file_info(std::string path, uint8_t depth);
  size_t file_size(const char *path);
  size_t file_size(std::string const &path);
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
  void set_power_ctrl_pin(GPIOPin *);

 protected:
  ErrorCode init_error_;
  uint8_t clk_pin_;
  uint8_t cmd_pin_;
  uint8_t data0_pin_;
  uint8_t data1_pin_;
  uint8_t data2_pin_;
  uint8_t data3_pin_;
  bool mode_1bit_;
  GPIOPin *power_ctrl_pin_{nullptr};

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

// Actions pour le streaming
template<typename... Ts> class SdMmcProcessFileAction : public Action<Ts...> {
 public:
  SdMmcProcessFileAction(SdMmc *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, path)
  TEMPLATABLE_VALUE(size_t, buffer_size)

  void set_process_callback(std::function<bool(const uint8_t*, size_t, size_t, size_t)> callback) {
    this->callback_ = std::move(callback);
  }

  void play(Ts... x) {
    auto path = this->path_.value(x...);
    auto buffer_size = this->buffer_size_.has_value() ? this->buffer_size_.value(x...) : DEFAULT_STREAM_BUFFER_SIZE;
    
    if (this->callback_) {
      this->parent_->process_file(path, this->callback_, buffer_size);
    }
  }

 protected:
  SdMmc *parent_;
  std::function<bool(const uint8_t*, size_t, size_t, size_t)> callback_;
};

// Actions traditionnelles
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
std::string memory_unit_to_string(MemoryUnits);
MemoryUnits memory_unit_from_size(size_t);
std::string format_size(size_t);

}  // namespace sd_mmc_card
}  // namespace esphome



