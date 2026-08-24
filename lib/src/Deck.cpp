#include "Deck.hpp"
#include "Contract.hpp"
#include "Error.hpp"
#include <expected>

namespace poker {

std::expected<std::pair<Card, Card>, std::error_code>
Deck::DealTwoCards() {
  std::pair<Card, Card> cards;
  {
    auto result = DrawRandomCard();
    if (!result)
      return std::unexpected(result.error());
    cards.first = result.value();
  }
  {
    auto result = DrawRandomCard();
    if (!result)
      return std::unexpected(result.error());
    cards.second = result.value();
  }
  return cards;
};

std::expected<Card, std::error_code> Deck::DrawRandomCard() {
  PRECOND(numCardsLeft_ >= 0, numCardsLeft_);
  PRECOND(numCardsLeft_ <= 52, numCardsLeft_);
  
  if (numCardsLeft_ == 0) {
    return std::unexpected(poker::errors::SimError::NoCardsLeft);
  };

  std::uniform_int_distribution<std::uint8_t> distr(0, numCardsLeft_ - 1);
  auto randNthCard = distr(gen_);
  
  ASSERT(randNthCard >= 0, randNthCard)
  ASSERT(randNthCard < DECK_SIZE, randNthCard);

  auto it = GetNthCardInDeck(deck_, randNthCard);

  ASSERT(it, "num cards left was not zero so we should have gotten a card from the deck");
  ASSERT(it->inDeck, "drawing a card form the deck expects the card to have been in the deck");
  it->inDeck = false;
  numCardsLeft_--;
  return it->value;
}

} // namespace poker