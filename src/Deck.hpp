#include <iostream>
#include <cstdlib>
#include <ctime>

#define DECK_SIZE 52

class Deck
{
	friend std::ostream& operator<<(std::ostream& os, Deck& d); // for debugging purposes

	public:
		Deck();
		~Deck();
		void generateDeck();
		void shuffle();
		char getTopCard();
	
	private:
		char deck[DECK_SIZE];
		int topCard;
};
