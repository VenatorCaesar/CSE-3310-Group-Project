#include "Deck.hpp"

Deck::Deck()
{
	topCard = DECK_SIZE-1;
	generateDeck();
}

//Nothing to delete
Deck::~Deck()
{
}

void Deck::generateDeck()
{
	int index = 0;
	
	for(int i = 1; i <= 4; i++)
	{
		for(int j = 1; j <= 0x0D; j++)
		{
			char suit = i << 4;
			deck[index] = (suit | j);
			index++;
		}
	}
	
	shuffle();
}

void Deck::shuffle()
{
	std::srand(std::time(nullptr));
	
	for(int i = 0; i < DECK_SIZE; i++)
	{
		int rand_idx = std::rand() % DECK_SIZE;
		char temp = deck[rand_idx];
		deck[rand_idx] = deck[i];
		deck[i] = temp;
	}
	
	return;
}

char Deck::getTopCard()
{
	return deck[topCard--];
}

std::ostream& operator<<(std::ostream& os, Deck& d)
{
	for(int i = 0; i < DECK_SIZE; i++)
	{
		os << "Card " << i << " " << d.deck[i] << std::endl;
	}
	
	return os;
}
