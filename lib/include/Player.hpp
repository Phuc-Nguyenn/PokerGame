#pragma once

#include <boost/asio.hpp>
#include <boost/beast/http/field.hpp>
#include <expected>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

#include "Card.hpp"
#include "Common.hpp"
#include "Connection.hpp"
#include "Error.hpp"
#include "Serializer.hpp"

namespace poker {

class Player {
public:

  Player(std::string_view player_id);

  template <int N> std::optional<Card> GetCard() {
    static_assert(N == 0 || N == 1, "Player has two cards.");

    if (!holeCards_.has_value()) {
      return std::nullopt;
    }

    if constexpr (N == 0)
      return holeCards_->first;
    else if constexpr (N == 1)
      return holeCards_->second;
  }

  void dealCards(std::pair<Card, Card> cards);

  /**
   * Prompts the player for the next action
   */
  std::expected<common::Action, errors::AppError>
  PromptNextAction(common::RoundState state);

  std::expected<void, errors::AppError>
  Connect(std::unique_ptr<Connection> new_connection);

  bool TakeMoney(common::TMoney amountToTake);

  void AddMoney(common::TMoney amountToAdd);
  bool HasMoney(common::TMoney amount) const;

  bool SetCurrentBet(common::TMoney currentBet);

  bool Folded() const { return folded_round_; };
  void FoldRound() { folded_round_ = true; };

  void AddToPotContribution(common::TMoney amountToAdd) { totalPotContribution_ += amountToAdd; }
  common::TMoney GetTotalPotContribution() const { return totalPotContribution_; }
  common::TMoney GetCurrentBet() const { return currentBet_; }

  void ResetRoundState();

private:
  std::optional<std::pair<Card, Card>> holeCards_{};
  common::TMoney money_{0};

  /**
   * The players action this round, they might not have played yet hence
   * optional
   */
  std::string player_id_{"unset_name"};
  std::optional<common::Action> action_;
  std::unique_ptr<Connection> connection_;
  poker::serializer::json::JsonSerializer serializer_;
  common::TMoney currentBet_{0};
  common::TMoney totalPotContribution_{0};

  bool folded_round_{false};
};
} // namespace poker