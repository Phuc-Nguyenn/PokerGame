#pragma once
#include <cstdint>
#include <ostream>
namespace poker {

enum class Suit : std::uint8_t {
  Unassigned = 0,
  Spades,
  Clubs,
  Diamond,
  Hearts
};

std::ostream &operator<<(std::ostream &o, Suit s);

enum class Rank : std::uint8_t {
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

std::ostream &operator<<(std::ostream &o, Rank r);

struct Card {
  Suit suit{};
  Rank rank{};
};

std::ostream &operator<<(std::ostream &o, Card c);


constexpr inline std::array<std::uint8_t, 14> rankToPrimeLut = {0, 2, 3, 5, 7,11, 13, 17, 19, 23, 29, 31, 37, 41};



} // namespace poker