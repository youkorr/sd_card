#pragma once
namespace esphome {
namespace custom_media_player {
#include "esphome.h"
#include "esphome/components/media_player/media_player.h"

class CustomMediaPlayer : public esphome::Component {
public:
  void setup() override {
    ESP_LOGI("CustomMediaPlayer", "Setup completed");
  }

  void play_audio(const std::string &file_path) {
    if (file_path.empty()) {
      ESP_LOGE("CustomMediaPlayer", "File path is empty!");
      return;
    }

    // Log the file path
    ESP_LOGI("CustomMediaPlayer", "Playing file: %s", file_path.c_str());

    // Use the ESPHome media_player to play the file
    auto *media = esphome::media_player::MediaPlayer::get("speaker");
    if (media != nullptr) {
      media->play_media(esphome::media_player::MEDIA_TYPE_MUSIC, file_path);
    } else {
      ESP_LOGE("CustomMediaPlayer", "Media player not found!");
    }
  }
};

}

}  // namespace custom_media_player
}  // namespace esphome
