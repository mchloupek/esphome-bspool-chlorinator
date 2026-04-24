#include "bs_pool.h"

#include "esphome/core/log.h"

namespace esphome {
namespace bs_pool {

static const char *const TAG = "bs_pool";

void BSPool::enqueue_packet(uint8_t b1, uint8_t b2, uint8_t b3) {
  if (!this->write_command_enabled_ && b1 != 0x3f) {
    ESP_LOGD(TAG, "Ignoring write command %02X %02X %02X (write_command_enabled=false)", b1, b2, b3);
    return;
  }
  if (!this->cell_current_query_enabled_ && b1 == 0x3f && b2 == FunctionCode::CELL_CURRENT_MEASUREMENT) {
    ESP_LOGD(TAG, "Ignoring CELL_CURRENT_MEASUREMENT query (cell_current_query_enabled=false)");
    return;
  }
  this->send_queue_.push_back({b1, b2, b3, 0, 0, 0, 0});
}

void BSPool::update() {
  for (auto &listener : this->listeners_)
    for (const FunctionCode code : listener->codes_to_poll())
      this->enqueue_packet('?', code, '\4');
}

void BSPool::loop() {
  while (this->available()) {
    if (this->read_array((uint8_t *)&this->buffer_, sizeof(this->buffer_))) {
      ESP_LOGD(TAG, "Received packet: %02X %02X %02X", this->buffer_.raw[0], this->buffer_.raw[1], this->buffer_.raw[2]);
      if (this->buffer_.raw[0] == 0x45 && this->buffer_.raw[1] == 0x52 && this->buffer_.raw[2] == 0x01)
      {
        this->error_counter++;
      }
      else
      {
        for (auto &listener : this->listeners_)
          listener->handle_message(this->buffer_);
      }
    } else {
      ESP_LOGW(TAG, "Junk on wire. Throwing away partial message");
      while (read() >= 0)
        ;
    }
  }

  if (this->error_counter > 3)
  {
    this->error_counter = 0;
    this->write_array({0x3f, 0x5a, 0x04, 0x00, 0x00, 0x00, 0x00});
    this->flush();
    ESP_LOGW(TAG, "Error recovery");
    return;
  }

  if (!this->send_queue_.empty()) {
    uint32_t now = ::millis();
    if (now - last_send_ms_ >= 20) {
      const auto &pkt = this->send_queue_.front();
      this->write_array(pkt);
      this->flush();
      ESP_LOGD(TAG, "Sending packet: %02X(%c) %02X(%c) %02X(%c) (MS: %u)", pkt[0],pkt[0], pkt[1],pkt[1], pkt[2],pkt[2], now);
      this->send_queue_.pop_front();
      last_send_ms_ = now;
    }
  }
}

}  // namespace bs_pool
}  // namespace esphome
