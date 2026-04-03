#include "bs_pool_number.h"

#include "esphome/core/log.h"

namespace esphome {
namespace bs_pool {

static const char *const TAG = "bs_pool.number";

static inline uint16_t get_u16(DataPacket &data) {
  return ((data.data_b3 << 8) | data.data_b2);
}

void BSPoolNumberComponent::dump_config() { ESP_LOGCONFIG(TAG, "BSPool Number:"); }

const std::vector<FunctionCode> BSPoolNumberComponent::codes_to_poll() {
  std::vector<FunctionCode> active_codes;
  if (this->power_number_ != nullptr)
    active_codes.push_back(FunctionCode::POWER);
  if (this->pool_volume_number_ != nullptr)
    active_codes.push_back(FunctionCode::POOL_VOLUME);
  if (this->cleaning_cycle_number_ != nullptr)
    active_codes.push_back(FunctionCode::CLEANING_CYCLE);
  if (this->ph_target_number_ != nullptr)
    active_codes.push_back(FunctionCode::PH_TARGET);
  if (this->orp_target_number_ != nullptr)
    active_codes.push_back(FunctionCode::ORP_TARGET);
  if (this->relay_delay_number_ != nullptr)
    active_codes.push_back(FunctionCode::RELAY_DELAY);
  if (this->program_1_start_number_ != nullptr)
    active_codes.push_back(FunctionCode::PROGRAM_1_START);
  if (this->program_1_stop_number_ != nullptr)
    active_codes.push_back(FunctionCode::PROGRAM_1_STOP);
  if (this->program_2_start_number_ != nullptr)
    active_codes.push_back(FunctionCode::PROGRAM_2_START);
  if (this->program_2_stop_number_ != nullptr)
    active_codes.push_back(FunctionCode::PROGRAM_2_STOP);
  return active_codes;
}

static void publish_program_time(number::Number *num, DataPacket &message) {
  if (num != nullptr) {
    float minutes_since_midnight = message.data_b3 * 60.0f + message.data_b2;
    num->publish_state(minutes_since_midnight);
  }
}

void BSPoolNumberComponent::handle_message(DataPacket &message) {
  switch (message.function_code) {
    case FunctionCode::POWER:
      if (this->power_number_ != nullptr)
        this->power_number_->publish_state(message.data_b2);
      break;
    case FunctionCode::POOL_VOLUME:
      if (this->pool_volume_number_ != nullptr)
        this->pool_volume_number_->publish_state(get_u16(message));
      break;
    case FunctionCode::CLEANING_CYCLE:
      if (this->cleaning_cycle_number_ != nullptr)
        this->cleaning_cycle_number_->publish_state(message.data_b2 * 10.0f);
      break;
    case FunctionCode::PH_TARGET:
      if (this->ph_target_number_ != nullptr)
        this->ph_target_number_->publish_state(get_u16(message) / 100.0f);
      break;
    case FunctionCode::ORP_TARGET:
      if (this->orp_target_number_ != nullptr) {
        uint16_t val = get_u16(message);
        // Mask out the free-chlorine flag bit if present
        this->orp_target_number_->publish_state(val & 0x7FF);
      }
      break;
    case FunctionCode::RELAY_DELAY:
      if (this->relay_delay_number_ != nullptr)
        this->relay_delay_number_->publish_state(message.data_b2);
      break;
    case FunctionCode::PROGRAM_1_START:
      publish_program_time(this->program_1_start_number_, message);
      break;
    case FunctionCode::PROGRAM_1_STOP:
      publish_program_time(this->program_1_stop_number_, message);
      break;
    case FunctionCode::PROGRAM_2_START:
      publish_program_time(this->program_2_start_number_, message);
      break;
    case FunctionCode::PROGRAM_2_STOP:
      publish_program_time(this->program_2_stop_number_, message);
      break;
  }
}

void BSPoolNumberComponent::send_command(uint8_t code, uint8_t b2, uint8_t b3) {
  this->parent_->write_array({code, b2, b3});
  this->parent_->flush();
}

void WritableNumber::control(float value) {
  this->publish_state(value);

  if (this->is_program_time_) {
    int total_minutes = (int) value;
    uint8_t hours = total_minutes / 60;
    uint8_t minutes = total_minutes % 60;
    this->parent_->send_command(this->code_, minutes, hours);
  } else if (this->is_16bit_) {
    uint16_t raw = (uint16_t)(value * this->scale_);
    this->parent_->send_command(this->code_, raw & 0xFF, (raw >> 8) & 0xFF);
  } else {
    uint8_t raw = (uint8_t)(value * this->scale_);
    this->parent_->send_command(this->code_, raw, 0x04);
  }
}

}  // namespace bs_pool
}  // namespace esphome
