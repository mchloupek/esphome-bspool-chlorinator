#include "bs_pool_switch.h"

#include <bitset>
#include <map>
#include <string>

#include "esphome/core/log.h"

namespace esphome {
namespace bs_pool {

static const char *const TAG = "bs_pool.switch";

void BSPoolSwitch::dump_config() { ESP_LOGCONFIG(TAG, "BSPool:"); }

const std::vector<FunctionCode> BSPoolSwitch::codes_to_poll() {
  std::vector<FunctionCode> active_codes;
  if (this->user_is_outdoor_switch_ != nullptr ||
      this->user_cover_switch_off_switch_ != nullptr ||
      this->user_flow_switch_installed_switch_ != nullptr ||
      this->user_orp_displayed_switch_ != nullptr ||
      this->user_ph_alarm_switch_ != nullptr ||
      this->user_ph_corrector_alkaline_switch_ != nullptr ||
      this->user_ph_control_switch_ != nullptr ||
      this->user_cover_installed_switch_ != nullptr)
    active_codes.push_back(FunctionCode::USER);
  if (this->stop_start_switch_ != nullptr)
    active_codes.push_back(FunctionCode::STOP_START);
  return active_codes;
}

void BSPoolSwitch::handle_message(DataPacket &message) {
  switch (message.function_code) {
    case FunctionCode::USER:
      if (this->user_is_outdoor_switch_ != nullptr)
        this->user_is_outdoor_switch_->publish_state(!(message.data_b2 & 0x01));
      if (this->user_cover_switch_off_switch_ != nullptr)
        this->user_cover_switch_off_switch_->publish_state(message.data_b2 & 0x02);
      if (this->user_flow_switch_installed_switch_ != nullptr)
        this->user_flow_switch_installed_switch_->publish_state(message.data_b2 & 0x04);
      if (this->user_orp_displayed_switch_ != nullptr)
        this->user_orp_displayed_switch_->publish_state(message.data_b2 & 0x08);
      if (this->user_ph_alarm_switch_ != nullptr)
        this->user_ph_alarm_switch_->publish_state(!(message.data_b2 & 0x10));
      if (this->user_ph_corrector_alkaline_switch_ != nullptr)
        this->user_ph_corrector_alkaline_switch_->publish_state(message.data_b2 & 0x20);
      if (this->user_ph_control_switch_ != nullptr)
        this->user_ph_control_switch_->publish_state(!(message.data_b2 & 0x40));
      if (this->user_cover_installed_switch_ != nullptr)
        this->user_cover_installed_switch_->publish_state(message.data_b2 & 0x80);
      break;
    case FunctionCode::STOP_START:
      // Protocol: 0=Start (running), 1=Stop (stopped)
      // Switch: ON=running, OFF=stopped
      if (this->stop_start_switch_ != nullptr)
        this->stop_start_switch_->publish_state(message.data_b2 == 0);
      break;
    default:
      break;
  }
}

static inline bool safe_state(switch_::Switch *sw, bool default_val = false) {
  return sw != nullptr ? sw->state : default_val;
}

void BSPoolSwitch::send_user_state() {
  UserSettings user_settings = {
      .is_outdoor = !safe_state(this->user_is_outdoor_switch_),
      .cover_switch_off = safe_state(this->user_cover_switch_off_switch_),
      .flow_switch_installed = safe_state(this->user_flow_switch_installed_switch_),
      .orp_displayed = safe_state(this->user_orp_displayed_switch_),
      .ph_alarm = !safe_state(this->user_ph_alarm_switch_),
      .ph_corrector_alkaline = safe_state(this->user_ph_corrector_alkaline_switch_),
      .ph_control = !safe_state(this->user_ph_control_switch_),
      .cover_installed = safe_state(this->user_cover_installed_switch_),
  };
  this->send_command(FunctionCode::USER,
                     *reinterpret_cast<uint8_t *>(&user_settings));
}

void BSPoolSwitch::send_command(uint8_t code, uint8_t b2, uint8_t b3) {
  this->parent_->enqueue_packet(code, b2, b3);
}

}  // namespace bs_pool
}  // namespace esphome
