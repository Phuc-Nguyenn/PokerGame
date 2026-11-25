#include "Error.hpp"

namespace poker {
namespace errors {

const std::error_category &GetSimErrorCategory() {
  static SimErrorCategory instance;
  return instance;
}

const std::error_category &GetClientErrorCategory() {
  static ClientErrorCategory instance;
  return instance;
}

const std::error_category &GetConfigErrorCategory() {
  static ConfigErrorCategory instance;
  return instance;
}

const std::error_category &GetConnectionErrorCategory() {
  static ConnectionErrorCategory instance;
  return instance;
}

std::string SimErrorCategory::message(int ev) const {
  switch (static_cast<SimError>(ev)) {
  case SimError::NoError:
    return "No error";
  case SimError::NoAction:
    return "No action";
  case SimError::StateNotHandled:
    return "State not handled";
  case SimError::NoCardsLeft:
    return "No cards left";
  case SimError::NoEligiblePlayersInShowdownTier:
    return "No eligible players in showdown tier";
  default:
    return "Unknown error";
  }
}

std::string ClientErrorCategory::message(int ev) const {
  switch (static_cast<ClientError>(ev)) {
  case ClientError::NoError:
    return "No error";
  case ClientError::BadRequest:
    return "Bad request";
  case ClientError::InvalidAction:
    return "Invalid action";
  case ClientError::RaiseLessThanBB:
    return "Raise amount is less than big blind";
  case ClientError::RaiseLessThanPrevRaise:
    return "Raise amount is less than previous raise";
  case ClientError::NotEnoughMoney:
    return "Not enough money";
  default:
    return "Unknown error";
  }
}

std::string ConfigErrorCategory::message(int ev) const {
  switch (static_cast<ConfigError>(ev)) {
  case ConfigError::NoError:
    return "No error";
  case ConfigError::FileOpenFailed:
    return "Failed to open configuration file";
  case ConfigError::InvalidConfig:
    return "Invalid configuration";
  case ConfigError::EmptyConfig:
    return "Empty configuration";
  case ConfigError::ParseError:
    return "Failed to parse configuration";
  case ConfigError::MissingField:
    return "Missing required configuration field";
  case ConfigError::InvalidFieldType:
    return "Invalid configuration field type";
  case ConfigError::InvalidFieldValue:
    return "Invalid configuration field value";
  default:
    return "Unknown error";
  }
}

std::string ConnectionErrorCategory::message(int ev) const {
  switch (static_cast<ConnectionError>(ev)) {
  case ConnectionError::NoError:
    return "No error";
  case ConnectionError::ResolveFailed:
    return "Failed to resolve host";
  case ConnectionError::ConnectFailed:
    return "Failed to connect to host";
  case ConnectionError::WriteFailed:
    return "Failed to write HTTP request";
  case ConnectionError::ReadFailed:
    return "Failed to read HTTP response";
  default:
    return "Unknown error";
  }
}

std::error_code make_error_code(poker::errors::SimError e) {
  return {static_cast<int>(e), poker::errors::GetSimErrorCategory()};
}

std::error_code make_error_code(poker::errors::ClientError e) {
  return {static_cast<int>(e), poker::errors::GetClientErrorCategory()};
}

std::error_code make_error_code(poker::errors::ConfigError e) {
  return {static_cast<int>(e), poker::errors::GetConfigErrorCategory()};
}

std::error_code make_error_code(poker::errors::ConnectionError e) {
  return {static_cast<int>(e), poker::errors::GetConnectionErrorCategory()};
}

} // namespace errors
} // namespace poker