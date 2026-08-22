#pragma once

#include "Card.hpp"
#include "Common.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <expected>
#include <system_error>

namespace poker {

static constexpr std::uint8_t DECK_SIZE = 52;

/**
 * Forward declarations
 */
class Player;

class Deck {

public:
  Deck() : rd_{}, gen_{rd_()} {
    for (Deck::IthCard i = 0; i < DECK_SIZE; ++i) {
      deck_.at(i) = {CardAtIdx(i), true};
    }
  };
  Deck(Deck &) = delete;
  Deck &operator=(Deck &) = delete;
  Deck(Deck &&) = delete;
  Deck &operator=(Deck &&) = delete;

  std::expected<common::functional::Void, std::error_code> DealPlayer(Player &player);

  std::expected<Card, std::error_code> DrawRandomCard();
  

  void Reset() {
    ResetDeck();
  };

  std::uint8_t GetNumCardsLeft(){ return numCardsLeft_; };

  using IthCard = std::array<Card, DECK_SIZE>::size_type;

private:
  struct DeckCard {
    Card value;
    bool inDeck;
  };

  using DeckType = std::array<DeckCard, DECK_SIZE>;
  DeckType deck_;
  static constexpr Card CardAtIdx(Deck::IthCard ith) {
    return Card{static_cast<Suit>((ith / 4) + 1),
                static_cast<Rank>((ith % 13) + 1)};
  };

  static constexpr DeckType::iterator GetNthCardInDeck(DeckType deck, IthCard card) {
    std::uint8_t count = 0;
    return std::ranges::find_if(
        deck,
        [&count = count, &nth = card](const bool &inDeck) {
          return inDeck && ++count == nth;
        },
        &DeckCard::inDeck);
  }

  std::uint8_t numCardsLeft_{DECK_SIZE};
  std::random_device rd_;
  std::mt19937 gen_;

  void ResetDeck()
  {
    std::ranges::for_each(deck_, [](auto& inDeck){inDeck = true;}, &DeckCard::inDeck);
    numCardsLeft_ = DECK_SIZE;
  };
};

} // namespace poker