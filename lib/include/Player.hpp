#pragma once

#include <boost/asio.hpp>
#include <boost/beast/http/field.hpp>
#include <expected>
#include <nlohmann/json.hpp>
#include <system_error>
#include <utility>

#include "Card.hpp"
#include "Common.hpp"
#include "Connection.hpp"
#include "Serializer.hpp"

namespace poker {

class Player {
public:

  Player(std::string_view player_id) : player_id_(player_id){
  }

  template <int N> Card &GetCard() {
    static_assert(N == 0 || N == 1, "Player has two cards.");
    if constexpr (N == 0)
      return holeCards_.first;
    else if constexpr (N == 1)
      return holeCards_.second;
  }

  void dealCards(std::pair<Card, Card> cards) { holeCards_ = std::move(cards); }

  /**
   * Prompts the player for the next action
   */
  std::expected<common::Action, std::error_code>
  PromptNextAction(common::RoundState state) {
    REQUIRES(!action_.has_value());
    REQUIRES(connection_ != nullptr);

    std::string_view path = "/next_move";

    auto bytes = serializer_.Serialize(state);

    auto result = connection_->SendRecv(bytes, "/next_move");

    if (!result.has_value())
    {
      SPDLOG_ERROR("Error occured when sending and receiving data for player=[{}]", player_id_);
      return std::unexpected(result.error());
    }

    serializer_.Deserialize(*result);

    ASSURES(action_.has_value());
    return {};
  };

  std::expected<common::functional::Void, std::error_code>
  Connect(std::unique_ptr<Connection> new_connection) {
    REQUIRES(!connection_);
    REQUIRES(new_connection != nullptr);
    connection_ = std::move(new_connection);
    return {};
  }

private:
  std::pair<Card, Card> holeCards_{};
  common::TMoney money_;

  /**
   * The players action this round, they might not have played yet hence
   * optional
   */
  std::string player_id_{"unset_name"};
  std::optional<common::Action> action_;
  std::unique_ptr<Connection> connection_;
  poker::serializer::json::JsonSerializer serializer_;
};
} // namespace poker