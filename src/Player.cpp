#include "Player.hpp"

Player::Player(std::string Name, int Age) : name{Name}, age{Age}
{
	for(int i = 0; i < HAND_SIZE; i++)
	{
		hand[i] = 0;
	}
	
	balance = 1000;
	pot = 0;
	amountBet = 0;
	minBetNeeded = 0;
	turn = 0;
	round = 0;
}

Player::~Player()
{
}

void Player::sortHand() 
{ 
    for (int i = 0; i < HAND_SIZE - 1; i++)  
    { 
        int min = i; 
        for (int j = i + 1; j < HAND_SIZE; j++)
		{
            if ((hand[min] & 0x0F) > (hand[j] & 0x0F)) 
			{
                min = j; 
			}
		}
  
        char key = hand[min]; 
        while (min > i)  
        { 
            hand[min] = hand[min - 1]; 
            min--; 
        } 
        hand[i] = key; 
    } 
} 

void Player::setHand(char new_hand[HAND_SIZE])
{
	for(int i = 0; i < HAND_SIZE; i++)
	{
		hand[i] = new_hand[i];
	}
}

void Player::replaceCard(int index, char new_card)
{
	hand[index] = new_card;
	sortHand();
}

char* Player::getHand()
{
	return hand;
}

std::string Player::getName()
{
	return name;
}

int Player::getAge()
{
	return age;
}

void Player::setUID(std::string UID)
{
	uid = UID;
}

std::string Player::getUID()
{
	return uid;
}

void Player::setTurnID(int turn)
{
	turnID = turn;
}

int Player::getTurnID()
{
	return turnID;
}

void Player::setScore(int new_score)
{
	score = new_score;
}

int Player::getScore()
{
	return score;
}

void Player::setBalance(int new_bal)
{
	balance = new_bal;
}

int Player::getBalance()
{
	return balance;
}

void Player::setPot(int new_pot)
{
	pot = new_pot;
}

int Player::getPot()
{
	return pot;
}

void Player::addAmountBet(int new_amount)
{
	amountBet += new_amount;
}

int Player::getAmountBet()
{
	return amountBet;
}
		
void Player::setMinBetNeeded(int new_amount)
{
	minBetNeeded = new_amount;
}
		
int Player::getMinBetNeeded()
{
	return minBetNeeded;
}

std::ostream& operator<<(std::ostream& os, Player& player)
{
	os << "Name is: " << player.name << std::endl
		<< "Age is: " << player.age << std::endl
		<< "UID is: " << player.uid << std::endl
		<< "Turn ID is: " << player.turnID << std::endl
		<< "Balance is: " << player.balance << std::endl
		<< "My hand is: ";
		
	for(int i = 0; i < HAND_SIZE; i++)
	{
		os << player.hand[i] << " ";
	}
	
	os << std::endl;
	
	return os;
}