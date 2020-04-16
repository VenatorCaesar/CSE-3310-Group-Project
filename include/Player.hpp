#include <iostream>

#ifndef PLAYER_HPP
#define PLAYER_HPP

#define HAND_SIZE 5

class Player
{
	friend std::ostream& operator<<(std::ostream& os, Player& player); // for debugging purposes

	public:
		Player(std::string Name, int Age);
		~Player();
		void sortHand();
		void setHand(char new_hand[HAND_SIZE]);
		void replaceCard(int index, char new_card);
		char* getHand();
		std::string getName();
		int getAge();
		void setUID(std::string UID);
		std::string getUID();
		void setTurnID(int turn);
		int getTurnID();
		void setScore(int new_score);
		int getScore();
		void setBalance(int new_bal);
		int getBalance();
		void setPot(int new_pot);
		int getPot();
		void addAmountBet(int new_amount);
		int getAmountBet();
		void setMinBetNeeded(int new_amount);
		int getMinBetNeeded();
		int round;
		int turn;
	
	private:
		std::string name;
		int age;
		std::string uid;//
		int turnID;//
		char hand[HAND_SIZE];//
		int score;
        int balance;//
		int pot;//
		int amountBet;
		int minBetNeeded;
};

#endif
