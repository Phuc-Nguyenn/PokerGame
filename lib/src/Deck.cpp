#include "Deck.hpp"
#include "Contract.hpp"
#include "Error.hpp"
#include <expected>
#include <format>

namespace poker {

Deck::Deck() : rd_{}, gen_{rd_()} {
  for (Deck::IthCard i = 0; i < DECK_SIZE; ++i) {
    deck_.at(i) = {CardAtIdx(i), true};
  }
}

void Deck::Reset() {
  ResetDeck();
}

std::uint8_t Deck::GetNumCardsLeft() {
  return numCardsLeft_;
}

void Deck::ResetDeck() {
  std::ranges::for_each(deck_, [](auto& inDeck){inDeck = true;}, &DeckCard::inDeck);
  numCardsLeft_ = DECK_SIZE;
}

std::expected<std::pair<Card, Card>, errors::AppError>
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

std::expected<Card, errors::AppError> Deck::DrawRandomCard() {
  PRECOND(numCardsLeft_ >= 0, numCardsLeft_);
  PRECOND(numCardsLeft_ <= 52, numCardsLeft_);
  
  if (numCardsLeft_ == 0) {
    return std::unexpected(errors::AppError(
        errors::SimError::NoCardsLeft,
        std::format("draw_card_failed num_cards_left={}", numCardsLeft_)));
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