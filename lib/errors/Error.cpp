#include "Error.hpp"

namespace poker {
namespace errors {

const std::error_category &GetSimErrorCategory() {
  static SimErrorCategory instance;
  return instance;
}

std::string SimErrorCategory::message(int ev) const {
  switch (static_cast<SimError>(ev)) {
  case SimError::BadRequest:
    return "Bad request";
  case SimError::InvalidAction:
    return "Invalid action";
  case SimError::NoCardsLeft:
    return "No cards left";
  case SimError::InvalidConfig:
    return "Invalid configuration";
  default:
    return "Unknown error";
  }
}

std::error_code make_error_code(poker::errors::SimError e) {
  return {static_cast<int>(e), poker::errors::GetSimErrorCategory()};
}

} // namespace errors
} // namespace poker