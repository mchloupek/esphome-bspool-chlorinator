#pragma once

#include <array>
#include <deque>

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace bs_pool {

enum FunctionCode : uint8_t {
  // Switches
  USER = 'r',
  STOP_START = 'S',
  // Select
  CONTROL_MODE = 'm',
  LANGUAGE = 'L',
  RELAY_STATUS = 'R',
  // Binary sensors
  ALARMS = 'A',
  WARNINGS = 'w',
  STATUS = 's',
  // Sensors
  CELL_CURRENT_MEASUREMENT = 'c',
  CELL_INTENSITY_MEASUREMENT = 'C',
  CELL_VOLTAGE_MEASUREMENT = 'V',
  PH_MEASUREMENT = 'p',
  SALT_CONCENTRATION_MEASUREMENT = 'N',
  TIME = 'H',
  SOFTWARE_VERSION = 'y',
  RADOX_MEASUREMENT = 'o',
  TEMPERATURE_MEASUREMENT = 'W',
  HOURS_LOW = 'F',
  HOURS_HIGH = 'G',
  CHLORINATOR_TYPE = 'Z',
  // Number (writable)
  POWER = 'T',
  POOL_VOLUME = 'v',
  CLEANING_CYCLE = 'b',
  PH_TARGET = 'P',
  ORP_TARGET = 'O',
  RELAY_DELAY = 'D',
  PROGRAM_1_START = 201,
  PROGRAM_1_STOP = 202,
  PROGRAM_2_START = 203,
  PROGRAM_2_STOP = 204,
};

union DataPacket {
  uint8_t raw[3];
  struct {
    uint8_t function_code;
    uint8_t data_b2;
    uint8_t data_b3;
  };
} __attribute__((packed));

class BSPoolListener {
 public:
  virtual void handle_message(DataPacket &message) = 0;
  virtual const std::vector<FunctionCode> codes_to_poll() = 0;
};

class BSPool : public uart::UARTDevice, public PollingComponent {
 public:
  void update() override;
  void loop() override;

  void register_listener(BSPoolListener *listener) {
    this->listeners_.push_back(listener);
  }

  void enqueue_packet(uint8_t b1, uint8_t b2, uint8_t b3);

 protected:
  DataPacket buffer_;
  uint32_t error_counter{0};
  bool write_command_enabled_{false};
  bool cell_current_query_enabled_{false};

  std::vector<BSPoolListener *> listeners_{};
  std::deque<std::array<uint8_t, 7>> send_queue_{};
  uint32_t last_send_ms_{0};
};

}  // namespace bs_pool
}  // namespace esphome
