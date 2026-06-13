#pragma once

namespace poker {

/**
 * Forward declarations
 */
class Player;

enum class Suit { UnAssigned = 0, Spades, Hearts, Diamond, Clubs };

enum class Rank {
  Unassigned = 0,
  Ace = 1,
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Jack = 11,
  Queen = 12,
  King = 13
};

struct Card {
Suit suit{};
Rank rank{};
};

class Deck {

public:
  void DealPlayer(Player &player);

private:

};
} // namespace poker