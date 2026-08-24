#include <gtest/gtest.h>

#include "Card.hpp"
#include "Deck.hpp"
#include "Error.hpp"

TEST(TestDeck, DeckGetIthCardInDeckReturnsTheIthIteratorForANewDeck)
{
	for (int i=0; i<poker::DECK_SIZE; ++i)
	{
		poker::Deck deck;
		auto it = deck.GetNthCardInDeck(deck.deck_, i);
		ASSERT_EQ(std::distance(deck.deck_.begin(), it), i);
	}
}

TEST(TestDeck, DeckStartsAtFull52AndResetsTo52)
{
	poker::Deck deck;

	deck.numCardsLeft_ = 0;	
	deck.Reset();
	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE);
}

TEST(TestDeck, DeckResetMakesAllCardsInDeck)
{
	poker::Deck deck;

	std::ignore = deck.DealTwoCards();
	std::ignore = deck.DrawRandomCard();

	deck.Reset();

	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE);
	for (const auto& card : deck.deck_) {
		ASSERT_TRUE(card.inDeck);
	}
}

TEST(TestDeck, DealingTwoCardsRemovesTwoFromDeck)
{
	poker::Deck deck;

	std::ignore = deck.DealTwoCards();
	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE - 2);
	std::ignore = deck.DealTwoCards();
	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE - 4);
}

TEST(TestDeck, DrawingSingleRandomCardRemovesOneFromDeck)
{
	poker::Deck deck;

	std::ignore = deck.DrawRandomCard();
	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE - 1);
	std::ignore = deck.DrawRandomCard();
	ASSERT_EQ(deck.GetNumCardsLeft(), poker::DECK_SIZE - 2);
}

TEST(TestDeck, DrawingSingleCards52TimesLeavesEmptyDeck)
{
	/** if its not 52, then someone has changed the deck size? why did someone change the deck size? */
	ASSERT_EQ(poker::DECK_SIZE, 52);

	poker::Deck deck;	
	for(int i=0; i<poker::DECK_SIZE; ++i)
	{
		std::ignore = deck.DrawRandomCard(); 
	}

	ASSERT_EQ(deck.GetNumCardsLeft(), 0);

	auto result = deck.DrawRandomCard();
	ASSERT_FALSE(result.has_value());
	ASSERT_EQ(result.error(), poker::errors::SimError::NoCardsLeft);
}

TEST(TestDeck, DrawingRandomCards52TimesAreAllDifferent)
{
	poker::Deck deck;
	std::set<std::pair<poker::Rank, poker::Suit>> seen;
	for(int i=0; i<poker::DECK_SIZE; ++i)
	{
		auto result = deck.DrawRandomCard();
		ASSERT_TRUE(result.has_value());
		auto card_as_pair = std::make_pair(result->rank, result->suit);
		ASSERT_FALSE(seen.contains(card_as_pair));
		seen.insert(card_as_pair);
	}
}

TEST(TestDeck, DrawingTwoCards26TimesLeavesEmptyDeck)
{
	ASSERT_EQ(poker::DECK_SIZE, 52);

	poker::Deck deck;	
	for(int i=0; i<poker::DECK_SIZE/2; ++i)
	{
		std::ignore = deck.DealTwoCards(); 
	}

	ASSERT_EQ(deck.GetNumCardsLeft(), 0);

	auto result = deck.DrawRandomCard();
	ASSERT_FALSE(result.has_value());
	ASSERT_EQ(result.error(), poker::errors::SimError::NoCardsLeft);
}

