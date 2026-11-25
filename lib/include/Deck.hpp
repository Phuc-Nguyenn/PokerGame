#pragma once

#include "Card.hpp"
#include "Error.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <expected>
#include <random>

namespace poker {

static constexpr std::uint8_t DECK_SIZE = 52;

/**
 * Forward declarations
 */
class Player;

class Deck {

public:
  Deck();
  Deck(Deck &) = delete;
  Deck &operator=(Deck &) = delete;
  Deck(Deck &&) = delete;
  Deck &operator=(Deck &&) = delete;

  std::expected<std::pair<Card, Card>, errors::AppError> DealTwoCards();

  std::expected<Card, errors::AppError> DrawRandomCard();
  
  void Reset();

  std::uint8_t GetNumCardsLeft();

  using IthCard = std::array<Card, DECK_SIZE>::size_type;
  
#ifndef UNIT_TEST
private:
#else
public:
#endif


  struct DeckCard {
    Card value;
    bool inDeck;
  };

  using DeckType = std::array<DeckCard, DECK_SIZE>;
  DeckType deck_{};
  static constexpr Card CardAtIdx(Deck::IthCard ith) {
    return Card{static_cast<Suit>((ith / 4) + 1),
                static_cast<Rank>((ith % 13) + 1)};
  };

  static constexpr DeckType::iterator GetNthCardInDeck(DeckType& deck, IthCard card) {
    std::uint8_t count = 0;
    return std::ranges::find_if(
        deck,
        [&count = count, &nth = card](const bool &inDeck) {
          return inDeck && count++ == nth;
        },
        &DeckCard::inDeck);
  }

  std::uint8_t numCardsLeft_{DECK_SIZE};
  std::random_device rd_;
  std::mt19937 gen_;

  void ResetDeck();
};

} // namespace poker