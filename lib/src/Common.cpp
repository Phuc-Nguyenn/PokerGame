#include "Common.hpp"
#include "contract.hpp"

namespace poker::common {

std::ostream& operator<<(std::ostream& o, Command c)
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

std::ostream& operator<<(std::ostream& o, Action a)
{
  o << a.command;
  if (a.command == Command::Raise)
    o << a.amount;
  return o;
}

std::ostream& operator<<(std::ostream& o, Stage s)
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

void RoundState::AdvanceStage() {
  REQUIRES(static_cast<int>(stage) < static_cast<int>(Stage::NumberOfStages));
  stage = static_cast<common::Stage>(static_cast<int>(stage) + 1);
  lastRaiseTo = 0;
}

void RoundState::Reset() {
  stage = common::Stage::PreDeal;
  pot = 0;
  communityCards.clear();
  prevActions.clear();
  lastRaiseTo = 0;
}

static Config readConfig()
{
	return Config{};
}

const Config &GetConfig() {
  static Config instance = readConfig();
  return instance;
}

} // namespace poker::common
