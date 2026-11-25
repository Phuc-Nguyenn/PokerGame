#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/span_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include "Error.hpp"
#include <boost/core/noncopyable.hpp>
#include <boost/system/detail/error_code.hpp>
#include <expected>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

class Connection : private boost::noncopyable {
public:
  Connection(boost::asio::any_io_executor executor, std::string host,
             std::uint16_t port);

  std::expected<std::vector<std::byte>, poker::errors::AppError> SendRecv(std::span<std::byte> payload, std::string_view path);

  boost::asio::any_io_executor executor_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::tcp_stream stream_;
  std::string host_;
  std::uint16_t port_;
};