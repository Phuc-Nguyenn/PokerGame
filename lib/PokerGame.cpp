#include "PokerGame.hpp"
#include "Contract.hpp"
#include "errors/Error.hpp"

#include <cassert>
#include <iostream>
#include <spdlog/spdlog.h>

namespace poker {

using PlayerIdx = PokerGame::PlayerIdx;

PokerGame::PokerGame() {}

void PokerGame::Init() {
  // ConnectPlayers();
}

std::expected<BoardState, std::error_code>
PokerGame::ConnectPlayers(BoardState state) {
  return BoardState::PreDeal;
}

std::expected<BoardState, std::error_code>
PokerGame::DealHoleCards(BoardState state) {
  REQUIRES(state == BoardState::PreDeal);
  REQUIRES(deck_.GetState() == DeckState::PreDeal);

  /**
   * Deal two cards to every player
   */
  PlayerIdx underTheGun = PlayerUnderTheGun(players_, dealerIdx_);
  PlayerIdx p = underTheGun;
  do {
    p %= players_.size();

    assert(p > 0 && p < players_.size());
    deck_.DealPlayer(players_.at(p));

    p++;
  } while (p != underTheGun);

  return BoardState::PreFlop;
}

std::expected<BoardState, std::error_code>
PokerGame::DoBettingRound(BoardState state) {

  REQUIRES(state == BoardState::PreFlop || state == BoardState::Flop ||
           state == BoardState::Turn || state == BoardState::River);

  do {
  } while(true);

  return {};
}

std::expected<BoardState, std::error_code>
PokerGame::DealCommunityCards(BoardState state, std::uint8_t numOfCards) {
  return {};
}

std::expected<BoardState, std::error_code> PokerGame::ShowDown(BoardState state) { return {}; }

std::expected<BoardState, std::error_code> PokerGame::ClearAndReset() {
  return {};
}

void PokerGame::Start() {
  try {
    while (true) {
      BoardState state{BoardState::PreDeal};

      // clang-format off
      auto result = 
              DealHoleCards(state)
              .and_then([this](BoardState s) { return DoBettingRound(s); })
              .and_then([this](BoardState s) { return DealCommunityCards(s, 3); })
              .and_then([this](BoardState s) { return DoBettingRound(s); })
              .and_then([this](BoardState s) { return DealCommunityCards(s, 1); })
              .and_then([this](BoardState s) { return DoBettingRound(s); })
              .and_then([this](BoardState s) { return DealCommunityCards(s, 1); })
              .and_then([this](BoardState s) { return DoBettingRound(s); })
              .and_then([this](BoardState s) { return ShowDown(s); });
      // clang-format on

      if (!result)
      {
        
      }

    }
  } catch (contract::exception &e) {
    std::cout << std::endl << e.what() << std::endl;
  }
}

PlayerIdx PokerGame::PlayerSmallBlind(const std::vector<Player> &players,
                                      const PlayerIdx dealerIdx) const {
  REQUIRES(players.size() > 0)
  REQUIRES(dealerIdx > 0 && dealerIdx < players.size());

  return (dealerIdx + 1) % players.size();
}

PlayerIdx PokerGame::PlayerUnderTheGun(const std::vector<Player> &players,
                                       const PlayerIdx dealerIdx) const {
  REQUIRES(players.size() > 0)
  REQUIRES(dealerIdx > 0 && dealerIdx < players.size());

  return (dealerIdx + 3) % players.size();
}

void PokerGame::Stop() {}

} // namespace poker