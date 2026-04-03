#pragma once

#include "../bs_pool.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace bs_pool {

class BSPoolNumberComponent;

class WritableNumber : public number::Number, public Parented<BSPoolNumberComponent> {
 public:
  void set_function_code(uint8_t code) { this->code_ = code; }
  void set_is_16bit(bool v) { this->is_16bit_ = v; }
  void set_scale(float s) { this->scale_ = s; }
  void set_is_program_time(bool v) { this->is_program_time_ = v; }

  uint8_t get_function_code() const { return this->code_; }
  bool get_is_16bit() const { return this->is_16bit_; }
  float get_scale() const { return this->scale_; }
  bool get_is_program_time() const { return this->is_program_time_; }

 protected:
  void control(float value) override;

  uint8_t code_{0};
  bool is_16bit_{false};
  float scale_{1.0f};
  bool is_program_time_{false};
};

class BSPoolNumberComponent : public BSPoolListener,
                              public Component,
                              public Parented<BSPool> {
 public:
  void dump_config() override;

  const std::vector<FunctionCode> codes_to_poll() override;
  void handle_message(DataPacket &message) override;

  void send_command(uint8_t code, uint8_t b2, uint8_t b3 = 0x04);

  SUB_NUMBER(power);
  SUB_NUMBER(pool_volume);
  SUB_NUMBER(cleaning_cycle);
  SUB_NUMBER(ph_target);
  SUB_NUMBER(orp_target);
  SUB_NUMBER(relay_delay);
  SUB_NUMBER(program_1_start);
  SUB_NUMBER(program_1_stop);
  SUB_NUMBER(program_2_start);
  SUB_NUMBER(program_2_stop);
};

}  // namespace bs_pool
}  // namespace esphome
