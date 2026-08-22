#include "PokerGame.hpp"
#include "Common.hpp"
#include "Contract.hpp"
#include "Deck.hpp"
#include "Error.hpp"
#include "Player.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <expected>
#include <fstream>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <system_error>

namespace poker {

using PlayerIdx = PokerGame::PlayerIdx;
using RoundState = common::RoundState;
using Stage = common::Stage;
using Void = common::functional::Void;

PokerGame::PokerGame() = default;

std::expected<config, std::error_code>
PokerGame::ReadConfig(const boost::filesystem::path &file) {
  std::ifstream f{file.string()};

  if (!f.is_open()) {
    int err = errno;
    std::error_code ec(err, std::generic_category());
    spdlog::critical("Unable to open file [{}] [{}]", file.string(),
                     ec.message());
    return std::unexpected(ec);
  }

  nlohmann::json json = nlohmann::json::parse(f, nullptr, false);
  config config;
  if (json.empty()) {
    return std::unexpected(errors::SimError::InvalidConfig);
  }

  auto get_config_field = [&](auto json, std::string_view key,
                              std::string_view err_msg) -> auto {
    try {
      return json.at(key);
    } catch (const std::exception &e) {
      spdlog::critical("Critical configuration error: [{}]", err_msg);
      throw;
    }
  };

  try {

    config.hostname = get_config_field(json, "hostname",
                                       "unable to get poker game's hostname");
    config.port = get_config_field(json, "port",
                                   "unable to get poker game's hosted port");

    auto player_configs =
        get_config_field(json, "players", "unable to get players");
    config.number_of_players = player_configs.size();

    for (auto player_config : player_configs) {
      PlayerConfig new_player{
          .name = get_config_field(player_config, "name",
                                   "unable to get player's name"),
          .hostname = get_config_field(
              player_config, "hostname",
              std::format(
                  "unable to get field=[hostname] from player [{}] config",
                  std::string(player_config.at("name")))),
          .port = get_config_field(
              player_config, "port",
              std::format("unable to get field=[port] from player [{}] config",
                          std::string(player_config.at("name")))),
      };
      config.players.push_back(new_player);
    }
  } catch (const std::exception &e) {
    spdlog::critical("Failed reading configuration: [{}]", e.what());
    return std::unexpected(errors::SimError::InvalidConfig);
  }

  spdlog::info("Configuration=[{}]", json.dump());

  return config;
}

std::expected<Void, std::error_code>
PokerGame::Init(const boost::filesystem::path &file) {
  {
    auto result = ReadConfig(file);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }

    spdlog::info("Successfully parsed configuration");
    config_ = result.value();
  }
  {
    for (auto i : std::views::iota(0, config_.number_of_players))
      players_.emplace_back(Player{});
  }
  {
    auto result = ConnectPlayers();
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
  }

  return {};
}

std::expected<common::functional::Void, std::error_code>
PokerGame::ConnectPlayers() {
  auto result = ForEachPlayer(
      [this](Player &player, const PlayerConfig &config)
          -> std::expected<common::functional::Void, std::error_code> {
        auto new_connection = std::make_unique<Connection>(
            this->ctx_.get_executor(), config.hostname, config.port);
        return player.Connect(std::move(new_connection));
      });

  return {};
}
std::expected<common::functional::Void, std::error_code>
PokerGame::DealHoleCards() {
  PRECOND(state_.stage == common::Stage::PreDeal, state_.stage);
  REQUIRES(deck_.GetNumCardsLeft() == DECK_SIZE);
  REQUIRES(players_.size() > 0)

  PlayerIdx startFrom = PlayerUnderTheGun();
  auto result = ForEachPlayer(
      [this](Player &player) { return deck_.DealPlayer(player); }, startFrom);

  ENSURES(result.has_value());
  return {};
}

/**
 * Performs a single betting round by
 *  1. checking the stage in 'state' is [PreFlop|Flop|Turn|River]
 *  2. begin at the under the gun or SB
 *  3. for each players, prompts for the next action, then validates, then
 * performs action
 */
std::expected<common::functional::Void, std::error_code>
PokerGame::DoBettingRound() {
  REQUIRES(state_.stage == common::Stage::PreFlop ||
           state_.stage == common::Stage::Flop ||
           state_.stage == common::Stage::Turn ||
           state_.stage == common::Stage::River);

  PlayerIdx startIdx =
      state_.stage == Stage::PreFlop ? PlayerUnderTheGun() : PlayerSmallBlind();

  auto result = ForEachPlayer(
      [this](Player &player) -> std::expected<Void, std::error_code> {
        // clang-format off
          auto result = player.PromptNextAction(state_)
                .and_then([this, &player](common::Action action) { return isValidAction(player, action, state_); })
                .and_then([this, &player](common::Action action) { return DoAction(player, action, state_); });
        // clang-format on

        if (!result) {
          return std::unexpected(result.error());
        }
        return std::expected<Void, std::error_code>{};
      },
      startIdx);

  if (!result) {
    return std::unexpected(result.error());
  }

  return {};
}

std::expected<common::Action, std::error_code>
PokerGame::isValidAction(const Player &player, const common::Action &action,
                         const RoundState &state) {
  return action;
}

std::expected<Void, std::error_code>
PokerGame::DoAction(Player &player, const common::Action &action,
                    const RoundState &stage) {
  return {};
}

std::expected<common::functional::Void, std::error_code>
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

  state_.AdvanceStage();
  return {};
}

std::expected<common::functional::Void, std::error_code> PokerGame::ShowDown() {
  return {};
}

std::expected<common::functional::Void, std::error_code>
PokerGame::ClearAndReset() {
  return {};
}

std::expected<Void, std::error_code> PokerGame::Start() {
  try {
    while (true) {
      state_ = {.stage = Stage::PreDeal, .pot = 0, .prevActions = {}};

      // clang-format off
      auto result = 
              DealHoleCards()
              .and_then([this](auto _) { return DoBettingRound(); })
              .and_then([this](auto _) { return DealCommunityCards(3); })
              .and_then([this](auto _) { return DoBettingRound(); })
              .and_then([this](auto _) { return DealCommunityCards(1); })
              .and_then([this](auto _) { return DoBettingRound(); })
              .and_then([this](auto _) { return DealCommunityCards(1); })
              .and_then([this](auto _) { return DoBettingRound(); })
              .and_then([this](auto _) { return ShowDown(); });
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

std::expected<Void, std::error_code> PokerGame::Stop() { return {}; };

} // namespace poker