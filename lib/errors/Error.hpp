#pragma once

#include <format>
#include <source_location>
#include <string>
#include <system_error>

namespace poker {
namespace errors {

struct AppError
{
  std::error_code code;
  std::string context;
  std::source_location location;
  bool is_retryable{false};

  template<typename AppErrorEnum> requires std::is_error_code_enum_v<AppErrorEnum>
  explicit AppError(AppErrorEnum code, std::string context, bool is_retryable = false,
                    std::source_location location = std::source_location::current())
      : code(code), context(std::move(context)), location(location), is_retryable(is_retryable) {
  }

  explicit AppError(std::error_code code, std::string context, bool is_retryable = false,
                    std::source_location location = std::source_location::current())
      : code(code), context(std::move(context)), location(location), is_retryable(is_retryable) {
  }

  std::string what() const {
    const auto& loc = location;
    return std::format("Error=[{}] Context=[{}] Function=[{}] Location=[{}:{}]", code.message(), context, loc.function_name(), loc.file_name(), loc.line());
  }

  bool IsRetryable() const { return is_retryable; }
};

enum class ConfigError {
  NoError = 0,
  FileOpenFailed,
  InvalidConfig,
  EmptyConfig,
  ParseError,
  MissingField,
  InvalidFieldType,
  InvalidFieldValue,
};

enum class ConnectionError {
  NoError = 0,
  ResolveFailed,
  ConnectFailed,
  WriteFailed,
  ReadFailed,
};

enum class ClientError {
  NoError = 0,
  BadRequest,
  InvalidAction,
  RaiseLessThanBB,
  RaiseLessThanPrevRaise,
  NotEnoughMoney,
};

enum class SimError {
  NoError = 0,
  NoAction,
  StateNotHandled,
  NoCardsLeft,
  NoEligiblePlayersInShowdownTier,
};

struct SimErrorCategory : std::error_category {
  const char *name() const noexcept override { return "Simulator Error"; }
  std::string message(int ev) const override;
};

struct ClientErrorCategory : std::error_category {
  const char *name() const noexcept override { return "Client Error"; }
  std::string message(int ev) const override;
};

struct ConfigErrorCategory : std::error_category {
  const char *name() const noexcept override { return "Configuration Error"; }
  std::string message(int ev) const override;
};

struct ConnectionErrorCategory : std::error_category {
  const char *name() const noexcept override { return "Connection Error"; }
  std::string message(int ev) const override;
};

const std::error_category &GetSimErrorCategory();
const std::error_category &GetClientErrorCategory();
const std::error_category &GetConfigErrorCategory();
const std::error_category &GetConnectionErrorCategory();

std::error_code make_error_code(poker::errors::SimError e);
std::error_code make_error_code(poker::errors::ClientError e);
std::error_code make_error_code(poker::errors::ConfigError e);
std::error_code make_error_code(poker::errors::ConnectionError e);

} // namespace errors
} // namespace poker

template <>
struct std::is_error_code_enum<poker::errors::SimError> : true_type {};

template <>
struct std::is_error_code_enum<poker::errors::ClientError> : true_type {};

template <>
struct std::is_error_code_enum<poker::errors::ConfigError> : true_type {};

template <>
struct std::is_error_code_enum<poker::errors::ConnectionError> : true_type {};