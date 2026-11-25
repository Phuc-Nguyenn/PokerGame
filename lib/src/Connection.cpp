
#include "Connection.hpp"

#include <format>
#include <nlohmann/json.hpp>

Connection::Connection(boost::asio::any_io_executor executor, std::string host,
                       std::uint16_t port)
    : executor_(executor), resolver_(executor), stream_(executor),
      host_(std::move(host)), port_(port) {}

std::expected<std::vector<std::byte>, poker::errors::AppError>
Connection::SendRecv(std::span<std::byte> payload, std::string_view path) {
  using poker::errors::AppError;
  using poker::errors::ConnectionError;

  /**
   * connect to remote endpoint
   */
  SPDLOG_DEBUG("resolving_endpoints host={} port={}", host_, port_);
  boost::system::error_code ec;
  auto endpoints = resolver_.resolve(host_, std::to_string(port_), ec);
  if (ec) {
    SPDLOG_ERROR("resolve_failed host={} port={} error={}", host_, port_, ec.message());
    return std::unexpected(AppError(
        ConnectionError::ResolveFailed,
        std::format("resolve_failed host={} port={} error={}", host_, port_, ec.message()),
        true));
  }

  SPDLOG_DEBUG("endpoints_resolved host={} port={}",
               endpoints->host_name(), endpoints->service_name());
  stream_.expires_after(std::chrono::seconds(1));

  stream_.connect(endpoints, ec);
  if (ec) {
    SPDLOG_ERROR("connect_failed host={} port={} error={}", host_, port_, ec.message());
    return std::unexpected(AppError(
        ConnectionError::ConnectFailed,
        std::format("connect_failed host={} port={} error={}", host_, port_, ec.message()),
        true));
  }

  SPDLOG_INFO("connection_established host={} port={}", host_, port_);

  /**
   * create http request
   */
  SPDLOG_DEBUG("creating_http_request host={} port={} path={}", host_, port_, path);
  boost::beast::http::request<boost::beast::http::buffer_body> req{
      boost::beast::http::verb::post, path, 11};

  req.set(boost::beast::http::field::host, host_);
  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(boost::beast::http::field::content_type, "application/json");

  req.body().data = payload.data();
  req.body().size = payload.size();
  req.content_length(payload.size());

  /**
   * send payload
   */
  SPDLOG_DEBUG("writing_http_request host={} port={} path={}", host_, port_, path);
  boost::beast::http::write(stream_, req, ec);
  if (ec) {
    SPDLOG_ERROR("http_write_failed host={} port={} path={} error={}", host_, port_, path, ec.message());
    return std::unexpected(AppError(
        ConnectionError::WriteFailed,
        std::format("http_write_failed host={} port={} path={} error={}", host_, port_, path, ec.message()),
        true));
  }

  /**
   * retrieve response
   */
  boost::beast::flat_buffer buffer;
  boost::beast::http::response<boost::beast::http::vector_body<std::byte>> res;
  SPDLOG_DEBUG("reading_http_response host={} port={} path={}", host_, port_, path);
  boost::beast::http::read(stream_, buffer, res, ec);
  if (ec) {
    SPDLOG_ERROR("http_read_failed host={} port={} path={} error={}", host_, port_, path, ec.message());
    return std::unexpected(AppError(
        ConnectionError::ReadFailed,
        std::format("http_read_failed host={} port={} path={} error={}", host_, port_, path, ec.message()),
        true));
  }

  return res.body();
}

