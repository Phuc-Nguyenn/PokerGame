#include <boost/asio/io_context.hpp>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <system_error>
#include <vector>

#include "Common.hpp"
#include "Deck.hpp"
#include "Error.hpp"
#include "Player.hpp"

namespace boost::filesystem {
class path;
}

namespace poker {
struct PlayerConfig {
  std::string name{};
  std::string hostname{};
  std::uint16_t port{};
};

struct config {
  std::string hostname{};
  int port{};
  int number_of_players{};
  uint32_t bb{};
  uint32_t sb{};
  std::vector<PlayerConfig> players{};
};

class PokerGame {
public:
  using PlayerIdx = std::vector<Player>::size_type;

  PokerGame();
  PokerGame(const PokerGame &) = delete;
  PokerGame &operator=(const PokerGame &) = delete;

  /**
   * Initialise the poker game
   */
  std::expected<void, errors::AppError>
  Init(const boost::filesystem::path &file);

  /**
   * Start the poker game's main loop which repetitively executes every round
   */
  std::expected<void, errors::AppError> Start();

  /**
   * Stop the poker game, allowing no more actions to be done, and closing of
   * connections Performs any nessecary cleanup logic
   */
  std::expected<void, errors::AppError> Stop();

private:
  std::expected<void, errors::AppError> ConnectPlayers();

  /**
   * Deal two cards to every player starting from under the gun
   */
  std::expected<void, errors::AppError> DealHoleCards();

  std::expected<void, errors::AppError> DoBettingRound();

  /**
   * Deal numOfCards amount of community cards
   * Advances the game stage in 'state' to next state by calling .AdvanceStage()
   *
   * REQUIRES(state.stage == Stage::PreFlop || state.stage == Stage::Flop ||
   * state.stage == Stage::Turn); REQUIRES(state.communityCards.size() +
   * numOfCards <= 5);
   */
  std::expected<void, errors::AppError>
  DealCommunityCards(std::uint8_t numOfCards);

  std::expected<void, errors::AppError> ShowDown();

  std::expected<void, errors::AppError> ClearAndReset();

  std::expected<void, errors::AppError> ForEachPlayer(
      std::function<
          std::expected<void, errors::AppError>(Player &, const PlayerConfig &)>
          function,
      const PlayerIdx startFrom = 0);

  std::expected<void, errors::AppError> ForEachPlayer(
      std::function<
          std::expected<void, errors::AppError>(Player &)>
          function,
      const PlayerIdx startFrom = 0, const bool skipFolded = false);

  static std::expected<common::Action, errors::AppError>
  isValidAction(const Player &player, const common::Action &action,
                const common::RoundState &state, const config& config);
                
  std::expected<void, errors::AppError>
  DoAction(Player &player, const common::Action &action);

  std::expected<config, errors::AppError>
  ReadConfig(const boost::filesystem::path &file);

  static std::int32_t EvaluatePlayerHand(const Player* player)
  {
    return 0;
  }

  void NextStage()
  {
    auto combinePlayerContributionToPot = [this](Player& player) {
      auto playerAmount = player.GetCurrentBet();
      state_.pot += playerAmount;
      player.TakeMoney(playerAmount);
      player.AddToPotContribution(playerAmount);
      player.SetCurrentBet(0);
      return std::expected<void, errors::AppError>();
    };
    std::ignore = ForEachPlayer(combinePlayerContributionToPot);
  
    state_.AdvanceStage();
  }

  boost::asio::io_context ctx_;
  Deck deck_;
  std::vector<Card> communityCards_;

  // do not sort or change the ordering as this order determines the seat order around the table TODO: possibly add a player seat id to make this more robust
  std::vector<std::unique_ptr<Player>> players_;
  PlayerIdx dealerIdx_;
  PlayerIdx PlayerSmallBlind() const;
  PlayerIdx PlayerUnderTheGun() const;
  config config_;
  common::RoundState state_;
};
}; // namespace poker
