#include <boost/asio/io_context.hpp>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <system_error>
#include <vector>

#include "Common.hpp"
#include "Deck.hpp"
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
  std::expected<common::functional::Void, std::error_code>
  Init(const boost::filesystem::path &file);

  /**
   * Start the poker game's main loop which repetitively executes every round
   */
  std::expected<common::functional::Void, std::error_code> Start();

  /**
   * Stop the poker game, allowing no more actions to be done, and closing of
   * connections Performs any nessecary cleanup logic
   */
  std::expected<common::functional::Void, std::error_code> Stop();

private:
  std::expected<common::functional::Void, std::error_code> ConnectPlayers();

  /**
   * Deal two cards to every player starting from under the gun
   */
  std::expected<common::functional::Void, std::error_code> DealHoleCards();

  std::expected<common::functional::Void, std::error_code> DoBettingRound();

  /**
   * Deal numOfCards amount of community cards
   * Advances the game stage in 'state' to next state by calling .AdvanceStage()
   *
   * REQUIRES(state.stage == Stage::PreFlop || state.stage == Stage::Flop ||
   * state.stage == Stage::Turn); REQUIRES(state.communityCards.size() +
   * numOfCards <= 5);
   */
  std::expected<common::functional::Void, std::error_code>
  DealCommunityCards(std::uint8_t numOfCards);

  std::expected<common::functional::Void, std::error_code> ShowDown();

  std::expected<common::functional::Void, std::error_code> ClearAndReset();

  std::expected<common::functional::Void, std::error_code> ForEachPlayer(
      std::function<
          std::expected<common::functional::Void, std::error_code>(Player &,const PlayerConfig& )>
          function,
      const PlayerIdx startFrom = 0) {
    auto p = startFrom;
    ASSERT(players_.size() > 0, players_.size());
    do {
      ASSERT(p >= 0, p);
      ASSERT(p < players_.size(),
             std::format("p=[{}], players.size=[{}]", p, players_.size()));
      auto result = function(players_.at(p), config_.players.at(p));
      if (!result)
        return std::unexpected(result.error());

      p = (p + 1) % players_.size();
    } while (p != startFrom);
    return {};
  };

  std::expected<common::functional::Void, std::error_code> ForEachPlayer(
      std::function<
          std::expected<common::functional::Void, std::error_code>(Player &)>
          function,
      const PlayerIdx startFrom = 0) {
    auto p = startFrom;
    ASSERT(players_.size() > 0, players_.size());
    do {
      ASSERT(p >= 0, p);
      ASSERT(p < players_.size(),
             std::format("p=[{}], players.size=[{}]", p, players_.size()));
      auto result = function(players_.at(p));
      if (!result)
        return std::unexpected(result.error());

      p = (p + 1) % players_.size();
    } while (p != startFrom);
    return {};
  };

  static std::expected<common::Action, std::error_code>
  isValidAction(const Player &player, const common::Action &action,
                const common::RoundState &state);
  std::expected<common::functional::Void, std::error_code>
  DoAction(Player &player, const common::Action &action,
           const common::RoundState &state);

  std::expected<config, std::error_code>
  ReadConfig(const boost::filesystem::path &file);

  boost::asio::io_context ctx_;
  Deck deck_;
  std::vector<Card> communityCards_;
  std::vector<Player> players_;
  PlayerIdx dealerIdx_;
  PlayerIdx PlayerSmallBlind() const;
  PlayerIdx PlayerUnderTheGun() const;
  config config_;
  common::RoundState state_;
};
}; // namespace poker
