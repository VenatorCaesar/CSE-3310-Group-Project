#include <iostream>
#include <map>
#include <iterator>

#define HAND_SIZE 5

/*
	Card Value and Suit Decoder
	
	Suits:
		0x10 = Hearts
		0x20 = Diamonds
		0x30 = Clubs
		0x40 = Spades
		
	Values:
		0x01 = Ace
		0x02 = 2
		0x03 = 3
		0x04 = 4
		0x05 = 5
		0x06 = 6
		0x07 = 7
		0x08 = 8
		0x09 = 9
		0x0A = 10
		0x0B = Jack
		0x0C = Queen
		0x0D = King
*/

int checkHighCard(char* hand)
{
	int score;
	if((hand[0] & 0x0F) == 0x01) // check to see if player has an ace
	{
		score = 0x0E; //return one higher than king cause ace > king
	}
	else
	{
		score = hand[HAND_SIZE-1] & 0x0F;
	}
	
	return score;
}

int checkPair(char* hand)
{
	std::map<char,char> possible; // creates a map to hold the value of the card, not the suit
	
	for(int i = 0; i < HAND_SIZE; i++)
	{
		possible[(hand[i] & 0x0F)]++; // for each value of a card, increment the number of times it is seen
	}
	
	int highest = 0;
	
	for(int i = 4; i >= 0; i--)
	{
		char val = hand[i] & 0x0F;
		
		if((possible[val] == 2) && (val > highest))
		{
			highest = val;
		}
		else if((val == 0x01) && (possible[val] == 2))
		{
			highest = val;
		}
	}
	
	if(highest)
	{
		return (highest + 0x0E); // 0x0E is the previous type of hand's highest score
	}
	else
	{
		return 0; //if no pair found return 0
	}
}

int checkTwoPair(char* hand)
{
	std::map<char,char> possible; // creates a map to hold the value of the card, not the suit
	
	for(int i = 0; i < HAND_SIZE; i++)
	{
		possible[(hand[i] & 0x0F)]++; // for each value of a card, increment the number of times it is seen
	}
	
	int counter = 0;
	int highest = 0;
	
	std::map<char,char>::iterator it = possible.begin(); // create an iterator to go over the map
	
	while(it != possible.end()) // loop to find the largest pair
	{
		if((it->second == 2))
		{
			counter++;
			if(it->first > highest)
			{
				highest = it->first;
			}
		}
		
		it++;
	}
	
	if(counter == 2)
	{
		return (highest + 0x1C);
	}
	else //if no double pair was found
	{
		return 0;
	}
}

int checkThreeOfAKind(char* hand)
{
	std::map<char,char> possible; // creates a map to hold the value of the card, not the suit
	
	for(int i = 0; i < HAND_SIZE; i++)
	{
		possible[(hand[i] & 0x0F)]++; // for each value of a card, increment the number of times it is seen
	}
	
	std::map<char,char>::iterator it = possible.begin(); // create an iterator to go over the map
	
	int highest = 0;
	
	while(it != possible.end()) // loop to find the largest pair
	{
		if((it->second == 3))
		{
			highest = it->first;
			if(highest == 0x01)
			{
				highest = 0x0E;
			}
		}
		
		it++;
	}
	
	if(highest)
	{
		return (highest + 0x2A);
	}
	else
	{
		return 0;
	}
}

int checkStraight(char* hand)
{	
	
	int highest = 0;
	for(int i = 1; i < HAND_SIZE; i++)
	{
		if((hand[i-1] & 0x0F) == 0x01)
		{
		}
		else if((hand[i] & 0x0F) != ((hand[i-1] & 0x0f) + 1))
		{
			return 0;
		}
	}
	
	if((hand[0] & 0x0F) == 0x01)
	{
		highest = 0x0E;
	}
	else
	{
		highest = hand[HAND_SIZE-1] & 0x0F;
	}
	
	return (highest + 0x38);
}

int checkFlush(char* hand)
{
	char suit = hand[0] & 0xF0;
	
	for(int i = 1; i < HAND_SIZE; i++)
	{
		if((hand[i] & 0xF0) != suit)
		{
			return 0;
		}
	}
	
	int highest = hand[HAND_SIZE-1] & 0x0F;
	
	if((hand[0] & 0x0F) == 0x01)
	{
		highest = 0x0E;
	}
	
	return (highest + 0x46);
}

int checkFullHouse(char* hand)
{
	bool tok = false;
	bool pair = false;
	
	std::map<char,char> possible; // creates a map to hold the value of the card, not the suit
	
	for(int i = 0; i < HAND_SIZE; i++)
	{
		possible[(hand[i] & 0x0F)]++; // for each value of a card, increment the number of times it is seen
	}
	
	std::map<char,char>::iterator it = possible.begin(); // create an iterator to go over the map
	
	while(it != possible.end()) // loop to find the largest pair
	{
		if((it->second == 3))
		{
			tok = true;
		}
		if((it->second == 2))
		{
			pair = true;
		}
		
		it++;
	}
	
	if(pair && tok)
	{
		if((hand[0] & 0x0F) == 0x01)
		{
			return (0x0E + 0x54);
		}
		else
		{
			return ((hand[HAND_SIZE-1] & 0x0F) + 0x54);
		}
	}
	else
	{
		return 0;
	}
}

int checkFourOfAKind(char* hand)
{
	std::map<char,char> possible; // creates a map to hold the value of the card, not the suit
	
	for(int i = 0; i < HAND_SIZE; i++)
	{
		possible[(hand[i] & 0x0F)]++; // for each value of a card, increment the number of times it is seen
	}
	
	std::map<char,char>::iterator it = possible.begin(); // create an iterator to go over the map
	
	int highest = 0;
	
	while(it != possible.end()) // loop to find the largest pair
	{
		if((it->second == 4))
		{
			highest = it->first;
		}
		
		it++;
	}
	
	if(highest == 0x01)
	{
		return (0x0E + 0x62);
	}
	else if(highest)
	{
		return (highest + 0x62);
	}
	else
	{
		return 0;
	}
}

int checkStraightFlush(char* hand)
{
	char suit = hand[0] & 0xF0;
	
	for(int i = 1; i < HAND_SIZE; i++)
	{
		if(((hand[i-1] & 0x0F) == 0x01) && ((hand[i] & 0xF0) == suit))
		{
		}
		else if(((hand[i] & 0x0F) != ((hand[i-1] & 0x0f) + 1)) && ((hand[i-1] & 0xF0) == suit))
		{
			return 0;
		}
	}
	
	int highest = hand[HAND_SIZE-1] & 0x0F;
	
	if((hand[0] & 0x0F) == 0x01)
	{
		highest = 0x0E;
	}
	
	return (highest + 0x70);
}

int checkRoyalFlush(char* hand)
{
	char suit = hand[0] & 0xF0;
	
	for(int i = 1; i < HAND_SIZE; i++)
	{
		if(((hand[i-1] & 0x0F) == 0x01) && ((hand[i] & 0x0F) == 0x0A) && ((hand[i] & 0xF0) == suit))
		{
		}
		else if(((hand[i] & 0x0F) != ((hand[i-1] & 0x0f) + 1)) && ((hand[i-1] & 0xF0) == suit))
		{
			return 0;
		}
	}
	
	return (0x7F);
}

int scoreHand(char* hand)
{
	if(checkRoyalFlush(hand))
	{
		return checkRoyalFlush(hand);
	}
	else if(checkStraightFlush(hand))
	{
		return checkStraightFlush(hand);
	}
	else if(checkFourOfAKind(hand))
	{
		return checkFourOfAKind(hand);
	}
	else if(checkFullHouse(hand))
	{
		return checkFullHouse(hand);
	}
	else if(checkFlush(hand))
	{
		return checkFlush(hand);
	}
	else if(checkStraight(hand))
	{
		return checkStraight(hand);
	}
	else if(checkThreeOfAKind(hand))
	{
		return checkThreeOfAKind(hand);
	}
	else if(checkTwoPair(hand))
	{
		return checkTwoPair(hand);
	}
	else if(checkPair(hand))
	{
		return checkPair(hand);
	}
	else
	{
		return checkHighCard(hand);
	}
}
