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
  case SimError::InvalidMove:
    return "Invalid move";
  default:
    return "Unknown error";
  }
}
} // namespace errors
} // namespace poker

namespace std {
std::error_code make_error_code(poker::errors::SimError e) {
  return {static_cast<int>(e), poker::errors::GetSimErrorCategory()};
}
} // namespace std