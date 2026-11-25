#include "Card.hpp"

namespace poker {

std::ostream &operator<<(std::ostream &o, Suit s) {
  switch (s) {
  case Suit::Unassigned:
    o << "Unassigned";
    break;
  case Suit::Spades:
    o << "Spades";
    break;
  case Suit::Clubs:
    o << "Clubs";
  case Suit::Diamond:
    o << "Diamond";
    break;
  case Suit::Hearts:
    o << "Hearts";
    break;
  }
  return o;
}

std::ostream &operator<<(std::ostream &o, Rank r) {
  switch (r) {
  case Rank::Unassigned:
  case Rank::Ace:
    o << 'A';
    break;
  case Rank::Ten:
    o << 'T';
    break;
  case Rank::Jack:
    o << 'J';
    break;
  case Rank::Queen:
    o << "Q";
    break;
  case Rank::King:
    o << "K";
    break;
  case Rank::Two:
  case Rank::Three:
  case Rank::Four:
  case Rank::Five:
  case Rank::Six:
  case Rank::Seven:
  case Rank::Eight:
  case Rank::Nine:
    o << static_cast<int>(r);
    break;
  }
  return o;
}

std::ostream &operator<<(std::ostream &o, Card c) {
  o << c.rank << " of " << c.suit;
  return o;
}

} // namespace poker
