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
#include <boost/core/noncopyable.hpp>
#include <boost/system/detail/error_code.hpp>
#include <expected>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <system_error>

class Connection : private boost::noncopyable {
public:
  Connection(boost::asio::any_io_executor executor, std::string host,
             std::uint16_t port)
      : executor_(executor), resolver_(executor), stream_(executor),
        host_(std::move(host)), port_(port) {};

  std::expected<std::vector<std::byte>, std::error_code> SendRecv(std::span<std::byte> payload, std::string_view path) {
    /**
     * connect to remote endpoint
     */
    SPDLOG_DEBUG("resolving endpoints for {}:{}", host_, port_);
    auto endpoints = resolver_.resolve(
        host_, std::to_string(port_));
    SPDLOG_DEBUG("endpoints resolved, connecting to {}:{}",
                  endpoints->host_name(), endpoints->service_name());
    stream_.expires_after(std::chrono::seconds(1));

    boost::system::error_code ec;
    stream_.connect(endpoints, ec);
    if (ec) {
      SPDLOG_ERROR("Failed to connect to {}:{}", host_, port_);
      return std::unexpected(ec);
    }
    
    SPDLOG_INFO("connected to {}:{}", host_, port_);

    /**
     * create http request
     */
    SPDLOG_DEBUG("creating http request");
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
    SPDLOG_DEBUG("writing http request to the socket");
    boost::beast::http::write(stream_, req);

    /**
     * retrieve response
     */
    boost::beast::flat_buffer buffer;
    boost::beast::http::response<boost::beast::http::vector_body<std::byte>>
        res;
    SPDLOG_DEBUG("reading http response from the stream");
    boost::beast::http::read(stream_, buffer, res);

    return res.body();
  };

  boost::asio::any_io_executor executor_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::tcp_stream stream_;
  std::string host_;
  std::uint16_t port_;
};