#include "Deck.hpp"
#include "Contract.hpp"
#include "Player.hpp"
#include "Error.hpp"
#include <expected>

namespace poker {

std::expected<common::functional::Void, std::error_code>
Deck::DealPlayer(Player &player) {
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
  player.dealCards(std::move(cards));
  return common::functional::Void{};
};

std::expected<Card, std::error_code> Deck::DrawRandomCard() {
  std::uniform_int_distribution<std::uint8_t> distr(0, numCardsLeft_ - 1);
  auto randNthCard = distr(gen_);
  ASSERT(randNthCard >= 0, randNthCard)
  ASSERT(randNthCard < DECK_SIZE, randNthCard);

  auto it = GetNthCardInDeck(deck_, randNthCard);

  if (it == deck_.end()) {
    return std::unexpected(errors::SimError::NoCardsLeft);
  };

  ASSERT(it->inDeck, it->inDeck);
  it->inDeck = false;
  numCardsLeft_--;
  return it->value;
}

} // namespace poker