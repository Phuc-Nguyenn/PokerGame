#include "PokerGame.hpp"
#include "Common.hpp"
#include "Deck.hpp"
#include "Error.hpp"
#include "Player.hpp"
#include "contract.hpp"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <expected>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <set>
#include <spdlog/spdlog.h>
#include <system_error>
#include <utility>

namespace poker {

using PlayerIdx = PokerGame::PlayerIdx;
using RoundState = common::RoundState;
using Stage = common::Stage;

PokerGame::PokerGame() = default;

namespace {

template <typename T>
std::expected<void, errors::AppError>
GetConfigField(const nlohmann::json &j, std::string_view key, T &out,
               std::string_view scope = {}) {
  if (!j.is_object()) {
    return std::unexpected(errors::AppError(
        errors::ConfigError::InvalidConfig,
        std::format("configuration is not a json object. scope=[{}]",
                    scope)));
  }

  if (!j.contains(key)) {
    return std::unexpected(errors::AppError(
        errors::ConfigError::MissingField,
        std::format("configuration is missing key=[{}]. scope=[{}]", key,
                    scope)));
  }

  try {
    out = j.at(key).get<T>();
    return {};
  } catch (const nlohmann::json::type_error &e) {
    return std::unexpected(
        errors::AppError(errors::ConfigError::InvalidFieldType,
                         std::format("configuration has invalid field type for "
                                     "key=[{}]. scope=[{}] error=[{}]",
                                     key, scope, e.what())));
  } catch (const std::exception &e) {
    return std::unexpected(errors::AppError(
        errors::ConfigError::InvalidConfig,
        std::format(
            "configuration read failed for key=[{}]. scope=[{}] error=[{}]",
            key, scope, e.what())));
  }
}

} // namespace

std::expected<config, errors::AppError>
PokerGame::ReadConfig(const boost::filesystem::path &file) {
  std::ifstream f{file.string()};

  if (!f.is_open()) {
    int err = errno;
    std::error_code ec(err, std::generic_category());
    return std::unexpected(
        errors::AppError(errors::ConfigError::FileOpenFailed,
                         std::format("failed opening configuration file at path=[{}] error=[{}]",
                                     file.string(), ec.message())));
  }

  nlohmann::json json = nlohmann::json::parse(f, nullptr, false);
  if (json.is_discarded()) {
    return std::unexpected(errors::AppError(
        errors::ConfigError::ParseError,
        std::format("failed parsing configuration file. path=[{}]", file.string())));
  }
  if (json.empty()) {
    return std::unexpected(
        errors::AppError(errors::ConfigError::EmptyConfig,
                         std::format("configuration file is empty. path=[{}]", file.string())));
  }

  config cfg{};
  nlohmann::json::array_t player_configs;
  const std::string root_scope = std::format("path={}", file.string());

  return GetConfigField(json, "hostname", cfg.hostname, root_scope)
      .and_then(
          [&] { return GetConfigField(json, "port", cfg.port, root_scope); })
      .and_then([&] { return GetConfigField(json, "bb", cfg.bb, root_scope); })
      .and_then([&] { return GetConfigField(json, "sb", cfg.sb, root_scope); })
      .and_then([&] {
        return GetConfigField(json, "players", player_configs, root_scope);
      })
      .and_then([&]() -> std::expected<config, errors::AppError> {
        cfg.number_of_players = static_cast<int>(player_configs.size());
        for (std::size_t i = 0; i < player_configs.size(); ++i) {
          const auto &player_json = player_configs[i];
          PlayerConfig new_player{};

          auto res = GetConfigField(player_json, "name", new_player.name,
                                    std::format("player index={}", i))
                         .and_then([&] {
                           return GetConfigField(
                               player_json, "hostname", new_player.hostname,
                               std::format("player={}", new_player.name));
                         })
                         .and_then([&] {
                           return GetConfigField(
                               player_json, "port", new_player.port,
                               std::format("player={}", new_player.name));
                         });

          if (!res) {
            return std::unexpected(res.error());
          }
          cfg.players.push_back(std::move(new_player));
        }

        SPDLOG_INFO("config_loaded path={} players={}", file.string(),
                    cfg.number_of_players);
        return cfg;
      });
}

std::expected<void, errors::AppError>
PokerGame::Init(const boost::filesystem::path &file) {
  {
    auto result = ReadConfig(file);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }

    SPDLOG_INFO("config_parsed path={}", file.string());
    config_ = result.value();
  }
  {
    for (const auto &player_config : config_.players) {
      players_.emplace_back(std::make_unique<Player>(player_config.name));
    }
  }
  {
    auto result = ConnectPlayers();
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
  }

  return {};
}

std::expected<void, errors::AppError> PokerGame::ForEachPlayer(
    std::function<std::expected<void, errors::AppError>(Player &,
                                                        const PlayerConfig &)>
        function,
    const PlayerIdx startFrom) {
  auto p = startFrom;
  ASSERT(players_.size() > 0, players_.size());
  do {
    ASSERT(p >= 0, p);
    ASSERT(p < players_.size(),
           std::format("p=[{}], players.size=[{}]", p, players_.size()));
    ASSERT(players_.at(p) != nullptr, std::format("player {} is null", p));
    auto result = function(*players_.at(p), config_.players.at(p));
    if (!result)
      return std::unexpected(result.error());

    p = (p + 1) % players_.size();
  } while (p != startFrom);
  return {};
}

std::expected<void, errors::AppError> PokerGame::ForEachPlayer(
    std::function<std::expected<void, errors::AppError>(Player &)> function,
    const PlayerIdx startFrom, const bool skipFolded) {
  auto p = startFrom;
  ASSERT(players_.size() > 0, players_.size());
  do {
    ASSERT(p >= 0, p);
    ASSERT(p < players_.size(),
           std::format("p=[{}], players.size=[{}]", p, players_.size()));
    ASSERT(players_.at(p) != nullptr, std::format("player {} is null", p));
    auto &player = players_.at(p);

    if (!player->Folded()) {
      const auto result = function(*player);
      if (!result.has_value())
        return std::unexpected(result.error());
    }

    p = (p + 1) % players_.size();
  } while (p != startFrom);
  return {};
}

std::expected<void, errors::AppError> PokerGame::ConnectPlayers() {
  auto result = ForEachPlayer([this](Player &player, const PlayerConfig &config)
                                  -> std::expected<void, errors::AppError> {
    auto new_connection = std::make_unique<Connection>(
        this->ctx_.get_executor(), config.hostname, config.port);
    return player.Connect(std::move(new_connection));
  });

  return result;
}
std::expected<void, errors::AppError> PokerGame::DealHoleCards() {
  PRECOND(state_.stage == common::Stage::PreDeal, state_.stage);
  REQUIRES(deck_.GetNumCardsLeft() == DECK_SIZE);
  REQUIRES(players_.size() > 0)

  PlayerIdx startFrom = PlayerUnderTheGun();
  auto result = ForEachPlayer(
      [this](Player &player) -> std::expected<void, errors::AppError> {
        auto cards = deck_.DealTwoCards();
        if (!cards.has_value()) {
          return std::unexpected(cards.error());
        }
        player.dealCards(cards.value());
        return {};
      },
      startFrom);

  if (!result.has_value()) {
    return std::unexpected(result.error());
  }

  NextStage();
  POSTCOND(state_.stage == Stage::PreFlop, state_.stage);
  return {};
}

/**
 * Performs a single betting round by
 *  1. checking the stage in 'state' is [PreFlop|Flop|Turn|River]
 *  2. begin at the under the gun or SB
 *  3. for each players, prompts for the next action, then validates, then
 * performs action
 */
std::expected<void, errors::AppError> PokerGame::DoBettingRound() {
  PRECOND(state_.stage == common::Stage::PreFlop ||
              state_.stage == common::Stage::Flop ||
              state_.stage == common::Stage::Turn ||
              state_.stage == common::Stage::River,
          state_.stage);

  PlayerIdx startIdx =
      state_.stage == Stage::PreFlop ? PlayerUnderTheGun() : PlayerSmallBlind();

  auto promptAndAct =
      [this](Player &player) -> std::expected<void, errors::AppError> {
    return player.PromptNextAction(state_)
        .and_then([this, &player](common::Action action) {
          return isValidAction(player, action, state_, config_);
        })
        .and_then([this, &player](common::Action action) {
          return DoAction(player, action);
        });
  };

  auto result = ForEachPlayer(promptAndAct, startIdx, true);
  if (!result) {
    return std::unexpected(result.error());
  }

  NextStage();
  return {};
}

std::expected<common::Action, errors::AppError>
PokerGame::isValidAction(const Player &player, const common::Action &action,
                         const RoundState &state, const config &config) {
  switch (action.command) {
  case common::Command::Fold:
    return action;
  case common::Command::CheckCall:
    if (!player.HasMoney(state.lastRaiseTo)) {
      return std::unexpected(errors::AppError(
          errors::ClientError::NotEnoughMoney,
          std::format("player does not have enough money to call. required amount=[{}]",
                      state.lastRaiseTo)));
    }
  case common::Command::Raise:
    if (action.amount < config.bb) {
      return std::unexpected(errors::AppError(
          errors::ClientError::RaiseLessThanBB,
          std::format("raise amount is below big blind. amount=[{}] bb=[{}]",
                      action.amount, config.bb)));
    }
    if (action.amount < 2 * state.lastRaiseTo) {
      return std::unexpected(errors::AppError(
          errors::ClientError::RaiseLessThanPrevRaise,
          std::format("raise amount is below minimum raise. amount=[{}] min raise=[{}]",
                      action.amount, 2 * state.lastRaiseTo)));
    }
  }

  return action;
}

std::expected<void, errors::AppError>
PokerGame::DoAction(Player &player, const common::Action &action) {
  switch (action.command) {
  case common::Command::Fold:
    player.FoldRound();
    break;
  case common::Command::CheckCall: {
    auto hasEnoughMoney = player.SetCurrentBet(state_.lastRaiseTo);
    ASSERT(
        hasEnoughMoney,
        "not enough money to check/call. should've checked in IsValidAction()");
  }
  case common::Command::Raise:
    state_.lastRaiseTo = action.amount;
    auto hasEnoughMoney = player.SetCurrentBet(state_.lastRaiseTo);
    ASSERT(hasEnoughMoney,
           "player is the one who raised, was checked in IsValidAction()");
    break;
  }
  state_.prevActions.emplace_back(action);
  return {};
}

std::expected<void, errors::AppError>
PokerGame::DealCommunityCards(std::uint8_t numOfCards) {
  REQUIRES(state_.stage == Stage::PreFlop || state_.stage == Stage::Flop ||
           state_.stage == Stage::Turn);
  REQUIRES(state_.communityCards.size() + numOfCards <=
           5); // all the cards we deal should be less than 5

  while (numOfCards-- > 0) {
    auto result = deck_.DrawRandomCard();
    if (!result)
      return std::unexpected(result.error());
    state_.communityCards.emplace_back(result.value());
  }

  NextStage();
  return {};
}

// get each player who hasn't folded, sort them by pot contribution, evaluate
// the hand's "score", then for each L_i pot contribution level determine the
// winner/s of that pot and distribute evenly.
std::expected<void, errors::AppError> PokerGame::ShowDown() {

  auto potContributionAscending = [](const auto &p1, const auto &p2) {
    return p1.get().GetCurrentBet() < p2.get().GetCurrentBet();
  };

  std::set<common::TMoney> potContibutionTiers =
      players_ | std::views::transform(&Player::GetTotalPotContribution) |
      std::ranges::to<std::set>();

  common::TMoney prevTier{0};
  for (const common::TMoney tier : potContibutionTiers) {
    if (tier == 0)
      continue;

    std::vector<Player *> eligiblePlayers;
    common::TMoney tierPotAmount{0};

    for (const auto &player : players_) {

      if (player->GetTotalPotContribution() < tier) {
        continue;
      }

      auto tierContribution = tier - prevTier;
      tierPotAmount += tierContribution;
      if (!player->Folded()) {
        eligiblePlayers.emplace_back(player.get());
      }
    }

    if (!eligiblePlayers.empty())
      return std::unexpected(errors::AppError(
          errors::SimError::NoEligiblePlayersInShowdownTier,
          std::format("no eligible players in showdown tier. tier=[{}] tier pot=[{}]", tier,
                      tierPotAmount)));

    if (eligiblePlayers.size() == 1) {
      eligiblePlayers.at(0)->AddMoney(tierPotAmount);
    } else {
      auto evaluatedHands = eligiblePlayers |
                            std::views::transform(EvaluatePlayerHand) |
                            std::ranges::to<std::vector>();
      auto max = std::ranges::max(evaluatedHands);
      auto tierWinners =
          std::views::zip(evaluatedHands, eligiblePlayers) |
          std::views::filter([&](auto z) { return std::get<0>(z) == max; }) |
          std::ranges::to<std::vector>();
      common::TMoney winAmount = tierPotAmount / std::ranges::size(tierWinners);

      for (auto [handScore, player] : tierWinners) {
        player->AddMoney(winAmount);
      }
    }

    prevTier = tier;
  }

  return {};
}

std::expected<void, errors::AppError> PokerGame::ClearAndReset() {
  state_.Reset();

  std::ignore = ForEachPlayer([&](Player &player) {
    player.ResetRoundState();
    return std::expected<void, errors::AppError>();
  });

  return {};
}

std::expected<void, errors::AppError> PokerGame::Start() {
  try {
    while (true) {
      state_ = {.stage = Stage::PreDeal,
                .pot = 0,
                .prevActions = {},
                .lastRaiseTo = 0};

      // clang-format off
      auto result = 
              DealHoleCards()
              .and_then([this]() { return DoBettingRound(); })
              .and_then([this]() { return DealCommunityCards(3); })
              .and_then([this]() { return DoBettingRound(); })
              .and_then([this]() { return DealCommunityCards(1); })
              .and_then([this]() { return DoBettingRound(); })
              .and_then([this]() { return DealCommunityCards(1); })
              .and_then([this]() { return DoBettingRound(); })
              .and_then([this]() { return ShowDown(); });
      // clang-format on

      if (!result) {
        return std::unexpected(result.error());
      }
    }
  } catch (contract::exception &e) {
    std::cerr << std::endl << e.what() << std::endl;
  }
  return {};
}

PlayerIdx PokerGame::PlayerSmallBlind() const {
  REQUIRES(players_.size() > 0)
  REQUIRES(dealerIdx_ > 0 && dealerIdx_ < players_.size());

  return (dealerIdx_ + 1) % players_.size();
}

PlayerIdx PokerGame::PlayerUnderTheGun() const {
  REQUIRES(players_.size() > 0)
  REQUIRES(dealerIdx_ > 0 && dealerIdx_ < players_.size());

  return (dealerIdx_ + 3) % players_.size();
}

std::expected<void, errors::AppError> PokerGame::Stop() { return {}; };

} // namespace poker