#include "bs_pool_select.h"

#include <map>
#include <string>

#include "esphome/core/log.h"

namespace esphome {
namespace bs_pool {

static const char *const TAG = "bs_pool.select";

void BSPoolSelect::dump_config() { ESP_LOGCONFIG(TAG, "BSPool:"); }

const std::vector<FunctionCode> BSPoolSelect::codes_to_poll() {
  std::vector<FunctionCode> active_codes;
  if (this->control_mode_select_ != nullptr)
    active_codes.push_back(FunctionCode::CONTROL_MODE);
  if (this->language_select_ != nullptr)
    active_codes.push_back(FunctionCode::LANGUAGE);
  if (this->relay_status_select_ != nullptr)
    active_codes.push_back(FunctionCode::RELAY_STATUS);
  return active_codes;
}

static void publish_select_by_index(select::Select *sel, uint8_t index, const char *name) {
  if (sel == nullptr) return;
  auto selected = sel->at(static_cast<size_t>(index));
  if (selected.has_value())
    sel->publish_state(selected.value());
  else
    ESP_LOGW(TAG, "Received unknown %s: %d", name, index);
}

void BSPoolSelect::handle_message(DataPacket &message) {
  switch (message.function_code) {
    case FunctionCode::CONTROL_MODE:
      publish_select_by_index(this->control_mode_select_, message.data_b2, "control mode");
      break;
    case FunctionCode::LANGUAGE:
      publish_select_by_index(this->language_select_, message.data_b2, "language");
      break;
    case FunctionCode::RELAY_STATUS:
      publish_select_by_index(this->relay_status_select_, message.data_b2, "relay status");
      break;
  }
}

void BSPoolSelect::send_command(uint8_t code, uint8_t b2, uint8_t b3) {
  this->parent_->enqueue_packet(code, b2, b3);
}

}  // namespace bs_pool
}  // namespace esphome
