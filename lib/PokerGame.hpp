#include <cstdint>
#include <expected>
#include <string>
#include <system_error>
#include <vector>

#include "Player.hpp"
#include "Deck.hpp"

namespace poker {

enum class BoardState {
  Init,
  PreDeal,
  PreFlop,
  Flop,
  Turn,
  River,
  ShowDown,
  StopPlay
};

class PokerGame {
public:
  using PlayerIdx = std::vector<Player>::size_type;

  PokerGame();
  PokerGame(const PokerGame &) = delete;
  PokerGame &operator=(const PokerGame &) = delete;

  void Init();
  void Start();
  void Stop();

private:
  std::expected<BoardState, std::error_code> ConnectPlayers(BoardState state);

  std::expected<BoardState, std::error_code> DealHoleCards(BoardState state);

  std::expected<BoardState, std::error_code> DoBettingRound(BoardState state);

  std::expected<BoardState, std::error_code>
  DealCommunityCards(BoardState state, std::uint8_t numOfCards);

  std::expected<BoardState, std::error_code> ShowDown();

  std::expected<BoardState, std::error_code> ClearAndReset();

  std::expected<BoardState, std::error_code> ForEachPlayer(const PlayerIdx start, auto&& function)
  {
    function();
  };

  Deck deck_;
  std::vector<Player> players_;
  PlayerIdx dealerIdx_;
  PlayerIdx PlayerSmallBlind(const std::vector<Player> &players,
                             const PlayerIdx dealerIdx) const;
  PlayerIdx PlayerUnderTheGun(const std::vector<Player> &players,
                              const PlayerIdx dealerIdx) const;
};
}; // namespace poker
