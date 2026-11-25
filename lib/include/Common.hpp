#pragma once
#include "Card.hpp"
#include <cmath>
#include <cstdint>
#include <ostream>
#include <vector>

namespace poker {
namespace common {

using TMoney = std::double_t; 

enum class Command { Fold, CheckCall, Raise };

std::ostream& operator<<(std::ostream& o, Command c);
struct Action {
  Command command;
  TMoney amount;
};

std::ostream& operator<<(std::ostream& o, Action a);

struct Config {
  common::TMoney bigBlind = 20;
  common::TMoney smallBlind = 10;
  std::uint8_t numberOfPlayers = 8;
};

const Config &GetConfig();

enum class Stage {
  Init,
  PreDeal,
  PreFlop,
  Flop,
  Turn,
  River,
  ShowDown,
  StopPlay,
  NumberOfStages,
};

std::ostream& operator<<(std::ostream& o, Stage s);

struct RoundState {
  common::Stage stage;
  common::TMoney pot;
  std::vector<Card> communityCards;
  std::vector<common::Action> prevActions;
  common::TMoney lastRaiseTo{0};

  void AdvanceStage();
  void Reset();
};

} // namespace common
} // namespace poker
