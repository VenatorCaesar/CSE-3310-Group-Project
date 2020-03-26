#include <iostream>

#define HAND_SIZE 5

class Player
{
	friend std::ostream& operator<<(std::ostream& os, Player& player); // for debugging purposes

	public:
		Player(std::string Name, int Age, std::string UID, int turn, int bal);
		~Player();
		void sortHand();
		void setHand(char new_hand[HAND_SIZE]);
		void replaceCard(int index, char new_card);
		char* getHand();
		std::string getName();
		int getAge();
		std::string getUID();
		void setTurnID(int turn);
		int getTurnID();
		void setScore(int new_score);
		int getScore();
		void setBalance(int new_bal);
		int getBalance();
	
	private:
		std::string name;
		int age;
		std::string uid;
		int turnID;
		char hand[HAND_SIZE];
		int score;
        int balance;
};
