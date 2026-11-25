#include "Player.hpp"
#include "Error.hpp"
#include "contract.hpp"
#include <format>
#include <spdlog/spdlog.h>

namespace poker {

Player::Player(std::string_view player_id) : player_id_(player_id) {}

void Player::dealCards(std::pair<Card, Card> cards) {
  holeCards_ = std::move(cards);
}

std::expected<common::Action, errors::AppError>
Player::PromptNextAction(common::RoundState state) {
  REQUIRES(!action_.has_value());
  REQUIRES(connection_ != nullptr);

  auto bytes = serializer_.Serialize(state);

  constexpr int kMaxAttempts = 3;
  std::expected<std::vector<std::byte>, errors::AppError> result =
      std::unexpected(errors::AppError(errors::SimError::NoAction, "uninitialized"));

  for (int attempt = 1; attempt <= kMaxAttempts; ++attempt) {
    result = connection_->SendRecv(bytes, "/next_move");
    if (result.has_value()) {
      break;
    }

    if (!result.error().IsRetryable() || attempt == kMaxAttempts) {
      return std::unexpected(result.error());
    }

    SPDLOG_WARN("prompt_action_retry player_id={} attempt={} max_attempts={} error={}",
                player_id_, attempt, kMaxAttempts, result.error().code.message());
  }

  auto action = serializer_.Deserialize(*result);
  if (!action) {
    return std::unexpected(errors::AppError(
        errors::ClientError::InvalidAction,
        std::format("failed to deserialize player action. player id=[{}]", player_id_)));
  }

  ASSURES(action_.has_value());
  return action.value();
}

std::expected<void, errors::AppError>
Player::Connect(std::unique_ptr<Connection> new_connection) {
  REQUIRES(!connection_);
  REQUIRES(new_connection != nullptr);
  connection_ = std::move(new_connection);
  return {};
}

bool Player::TakeMoney(common::TMoney amountToTake) {
  if (amountToTake > money_)
    return false;
  money_ -= amountToTake;
  return true;
}

void Player::AddMoney(common::TMoney amountToAdd) {
  money_ += amountToAdd;
}

bool Player::HasMoney(common::TMoney amount) const
{
  return amount <= money_;
}

bool Player::SetCurrentBet(common::TMoney newCurrentBet)
{
  currentBet_ = std::min(newCurrentBet, money_);
  return newCurrentBet <= money_;
}

void Player::ResetRoundState()
{
  holeCards_.reset();
  action_.reset();
  currentBet_ = 0;
  totalPotContribution_ = 0;
  folded_round_ = false;
}

} // namespace poker
