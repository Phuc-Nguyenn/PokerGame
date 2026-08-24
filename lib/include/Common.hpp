#pragma once
#include "Card.hpp"
#include "Contract.hpp"
#include <cstdint>
#include <ostream>
#include <vector>

namespace poker {
namespace common {

using TMoney = std::uint64_t; 

enum class Command { Fold, CheckCall, Raise };

inline std::ostream& operator<<(std::ostream& o, Command c)
{
  switch(c)
  {
  case Command::Fold:
    o << "Fold";
    break;
  case Command::CheckCall:
    o << "Check/Call";
    break;
  case Command::Raise:
    o << "Raise";  
    break;
  default:
    o << "Unknown Command";
  }
  return o;
}
struct Action {
  Command command;
  TMoney amount;
};

inline std::ostream& operator<<(std::ostream& o, Action a)
{
  o << a.command;
  if (a.command == Command::Raise)
    o << a.amount;
  return o;
}

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

inline std::ostream& operator<<(std::ostream& o, Stage s)
{
  switch(s){
  case Stage::Init:
    o << "Init"; break;
  case Stage::PreDeal:
    o << "PreDeal"; break;
  case Stage::PreFlop:
    o << "PreFlop"; break;
  case Stage::Flop:
    o << "Flop"; break;
  case Stage::Turn:
    o << "Turn"; break;
  case Stage::River:
    o << "River"; break;
  case Stage::ShowDown:
    o << "ShowDown"; break;
  case Stage::StopPlay:
    o << "StopPlay"; break;
  case Stage::NumberOfStages:
    break;
  }
  return o;
}

struct RoundState {
  common::Stage stage;
  common::TMoney pot;
  std::vector<Card> communityCards;
  std::vector<common::Action> prevActions;

  void AdvanceStage() {
    REQUIRES(static_cast<int>(stage) < static_cast<int>(Stage::NumberOfStages));    
    stage = static_cast<common::Stage>(static_cast<int>(stage) + 1);
  }
};


namespace functional
{
struct Void{};
}


} // namespace common
} // namespace poker
