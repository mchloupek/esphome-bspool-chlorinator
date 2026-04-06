#include "bs_pool_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace bs_pool {

static const char *const TAG = "bs_pool.sensor";

static inline uint16_t get_u16(DataPacket &data) {
  return ((data.data_b3 << 8) | data.data_b2);
}

void BSPoolSensor::dump_config() { ESP_LOGCONFIG(TAG, "BSPool:"); }

const std::vector<FunctionCode> BSPoolSensor::codes_to_poll() {
  std::vector<FunctionCode> active_codes;
  if (this->cell_current_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::CELL_CURRENT_MEASUREMENT);
  if (this->cell_intensity_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::CELL_INTENSITY_MEASUREMENT);
  if (this->cell_voltage_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::CELL_VOLTAGE_MEASUREMENT);
  if (this->ph_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::PH_MEASUREMENT);
  if (this->salt_concentration_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::SALT_CONCENTRATION_MEASUREMENT);
  if (this->radox_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::RADOX_MEASUREMENT);
  if (this->temperature_sensor_ != nullptr)
    active_codes.push_back(FunctionCode::TEMPERATURE_MEASUREMENT);
  if (this->hours_of_operation_sensor_ != nullptr) {
    active_codes.push_back(FunctionCode::HOURS_LOW);
    active_codes.push_back(FunctionCode::HOURS_HIGH);
  }
  return active_codes;
}

void BSPoolSensor::handle_message(DataPacket &message) {
  switch (message.function_code) {
    case FunctionCode::CELL_CURRENT_MEASUREMENT:
      if (this->cell_current_sensor_ != nullptr)
        this->cell_current_sensor_->publish_state(get_u16(message) / 10.0f);
      break;
    case FunctionCode::CELL_INTENSITY_MEASUREMENT:
      if (this->cell_intensity_sensor_ != nullptr)
        this->cell_intensity_sensor_->publish_state(message.data_b2);
      break;
    case FunctionCode::CELL_VOLTAGE_MEASUREMENT:
      if (this->cell_voltage_sensor_ != nullptr)
        this->cell_voltage_sensor_->publish_state(get_u16(message) / 10.0f);
      break;
    case FunctionCode::PH_MEASUREMENT:
      if (this->ph_sensor_ != nullptr) {
        uint16_t ph = get_u16(message);
        if (ph != 0xFFFF)
          this->ph_sensor_->publish_state(ph / 100.0f);
        else
          this->ph_sensor_->publish_state(NAN);
      }
      break;
    case FunctionCode::SALT_CONCENTRATION_MEASUREMENT:
      if (this->salt_concentration_sensor_ != nullptr) {
        uint16_t salt_concentration = get_u16(message);
        if (salt_concentration != 0xFFFF)
          this->salt_concentration_sensor_->publish_state(salt_concentration / 100.0f);
        else
          this->salt_concentration_sensor_->publish_state(NAN);
      }
      break;
    case FunctionCode::RADOX_MEASUREMENT:
      if (this->radox_sensor_ != nullptr)
        this->radox_sensor_->publish_state(get_u16(message));
      break;
    case FunctionCode::TEMPERATURE_MEASUREMENT:
      if (this->temperature_sensor_ != nullptr) {
        if (message.data_b3 == 1) {
          // Sign byte 1 with any value — treat as sensor not connected
          this->temperature_sensor_->publish_state(NAN);
        } else {
          this->temperature_sensor_->publish_state(message.data_b2);
        }
      }
      break;
    case FunctionCode::HOURS_LOW:
      if (this->hours_of_operation_sensor_ != nullptr) {
        this->hours_minutes_ = message.data_b2;
        this->hours_low_ = message.data_b3;
        this->hours_low_received_ = true;
      }
      break;
    case FunctionCode::HOURS_HIGH:
      if (this->hours_of_operation_sensor_ != nullptr && this->hours_low_received_) {
        uint32_t total_hours = ((uint32_t)message.data_b3 << 16) | ((uint32_t)message.data_b2 << 8) | this->hours_low_;
        float hours = total_hours + this->hours_minutes_ / 60.0f;
        this->hours_of_operation_sensor_->publish_state(hours);
        this->hours_low_received_ = false;
      }
      break;
  }
}

}  // namespace bs_pool
}  // namespace esphome
