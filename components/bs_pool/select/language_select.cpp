#include "language_select.h"

namespace esphome {
namespace bs_pool {

void LanguageSelect::control(const std::string &value) {
  auto index = this->index_of(value);
  if (index.has_value()) {
    this->publish_state(value);
    this->parent_->send_command(FunctionCode::LANGUAGE,
                                static_cast<uint8_t>(index.value()));
  }
}

}  // namespace bs_pool
}  // namespace esphome
