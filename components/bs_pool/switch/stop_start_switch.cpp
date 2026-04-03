#include "stop_start_switch.h"

namespace esphome {
namespace bs_pool {

void StopStartSwitch::write_state(bool state) {
  this->publish_state(state);
  // Protocol: 0=Start (running), 1=Stop (stopped)
  // Switch: ON=running, OFF=stopped
  this->parent_->send_command(FunctionCode::STOP_START, state ? 0 : 1);
}

}  // namespace bs_pool
}  // namespace esphome
