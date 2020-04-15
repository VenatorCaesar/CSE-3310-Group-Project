#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <string>
#include "asio.hpp"
#include "chat_message.hpp"
#include "Deck.hpp"
#include "Player.hpp"
#include "Poker.hpp"
#include "json.hpp"


#define RAISE 0//
#define CALL 1//
#define CHECK 2//
#define FOLD 3
#define BUYIN 4//
#define TRADE 5//
#define ALLIN 6//
#define NEW_PLAYER 7//
#define DEL_PLAYER 8//
#define UID 9//
#define HANDS 10//
#define GAME_OVER 11
#define NEW_TURN 12//
#define NEW_HAND 13//

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
	public:
  		virtual ~chat_participant() {}
  		virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
	public:
  		void join(chat_participant_ptr participant)
  		{
    			participants_.insert(participant);
    			for (auto msg: recent_msgs_)
				{
      				participant->deliver(msg); // CSE3310 This is where previous chat messages are sent to new clients that have connected
				}
  		}

  		void leave(chat_participant_ptr participant)
  		{
    			participants_.erase(participant);
  		}

  		void deliver(const chat_message& msg)
  		{
    			recent_msgs_.push_back(msg);
    			while (recent_msgs_.size() > max_recent_msgs)
			{
      				recent_msgs_.pop_front(); // if the number of stored messages exceeds the limit, remove the last few ones
			}

    			for (auto participant: participants_)
			{
      				participant->deliver(msg); // CSE3310 This is where the message is sent to each client connected
			}
  		}		

		private:
  			std::set<chat_participant_ptr> participants_;
  			enum { max_recent_msgs = 100 }; // CSE3310 This is the number of maximum stored messages
  			chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session : public chat_participant, public std::enable_shared_from_this<chat_session>
{
	public:
		static int uids;
  		chat_session(tcp::socket socket, chat_room& room): socket_(std::move(socket)),room_(room)
  		{
			game_on = false;
			turn = 1;
			//uids = 1000;
			round = 0;
  		}

  		void start()
  		{
    			room_.join(shared_from_this());
    			do_read_header();
  		}

  		void deliver(const chat_message& msg)
  		{
    			bool write_in_progress = !write_msgs_.empty();
    			write_msgs_.push_back(msg);
    			if (!write_in_progress)
    			{
      				do_write();
    			}
  		}

	private:
  		void do_read_header()
  		{
    			auto self(shared_from_this());
    			asio::async_read(socket_,asio::buffer(read_msg_.data(), chat_message::header_length),[this, self](std::error_code ec, std::size_t /*length*/)
        		{
          			if (!ec && read_msg_.decode_header())
          			{
            				do_read_body();
          			}
          			else
          			{
            				room_.leave(shared_from_this());
          			}
 			});
  		}
		
		void game_start()
		{
			//rock on
			game_on = true;
			deck = new Deck();
										
			//create a vector of char* to hold the hands
			std::vector<char*> hands;
										
			for(unsigned int i = 0; i < players.size(); i++) // for each player create a char* of size 5
			{
				char hand[5] = {0,0,0,0,0};
				hands.push_back(hand); // add the char* to the vector
			}
			
			//adds 1 card to each hand 5 times
			for(int i = 0; i < 5; i++)
			{
				for(auto hand : hands)
				{
					hand[i] = deck->getTopCard();
				}
			}
			
			//set the players' hands and turn id, pot
			for(unsigned int i = 0; i < players.size(); i++)
			{
				players.at(i)->setHand(hands.at(i));
				players.at(i)->sortHand();
				players.at(i)->setTurnID(i+1);
				
				nlohmann::json::object_t object_value = {{"action",HANDS},{"uid",players.at(i)->getUID()},{"hand",players.at(i)->getHand()},{"turnID",players.at(i)->getTurnID()},{"round",round}}; // configure json object with parameters
				nlohmann::json j_object_value(object_value); // add it to the json
				
				std::stringstream ss;
				ss << j_object_value; // move the json into a string format
				std::string js = ss.str(); // put it into a string
				
				chat_message msg;
				msg.body_length(std::strlen(js.c_str()));// encode the string
				std::memcpy(msg.body(),js.c_str(), msg.body_length());
				msg.encode_header();
									
				room_.deliver(msg); // delivers message to everyone connected
			}
		}
		
		void game_over()
		{
			int high_score = 0;
			Player* highest_scoring_player;
			bool tie = false;
			int i = 1;
			
			for(Player* p : players)
			{
				p->setScore(scoreHand(p->getHand()));
				if(p->getScore() > high_score)
				{
					highest_scoring_player = p;
					high_score = p->getScore();
					tie = false;
					i = 1;
				}
				else if(p->getScore() == high_score)
				{
					tie = true;
					i++;
				}
			}
			
			int action = GAME_OVER;
			
			if(tie)
			{	
				std::string uidstrings[i];
				int j = 0;
				
				for(Player* p : players)
				{
					if(p->getScore() == high_score)
					{
						uidstrings[j] = p->getUID();
						j++;
					}
				}
				
				nlohmann::json::object_t object_value = {{"action",action},{"tie",tie},{"uid1",uidstrings[0]},{"uid2",uidstrings[1]},{"uid3",uidstrings[2]},{"uid4",uidstrings[3]},{"uid5",uidstrings[4]}}; // configure json object with parameters
				nlohmann::json j_object_value(object_value); // add it to the json
				
				std::stringstream ss;
				ss << j_object_value; // move the json into a string format
				std::string js = ss.str(); // put it into a string
				
				chat_message msg;
				msg.body_length(std::strlen(js.c_str()));// encode the string
				std::memcpy(msg.body(),js.c_str(), msg.body_length());
				msg.encode_header();
									
				room_.deliver(msg); // delivers message to everyone connected
			}
			else
			{
				nlohmann::json::object_t object_value = {{"action",action},{"tie",tie},{"uid",highest_scoring_player->getUID()}}; // configure json object with parameters
				nlohmann::json j_object_value(object_value); // add it to the json
				
				std::stringstream ss;
				ss << j_object_value; // move the json into a string format
				std::string js = ss.str(); // put it into a string
				
				chat_message msg;
				msg.body_length(std::strlen(js.c_str()));// encode the string
				std::memcpy(msg.body(),js.c_str(), msg.body_length());
				msg.encode_header();
									
				room_.deliver(msg); // delivers message to everyone connected
			}
			
			game_on = false;
			
			for(auto it = standbyPlayers.begin(); it != standbyPlayers.end(); it++)
			{
				Player* p = *it;
				
				standbyPlayers.erase(it);
				players.push_back(p);
			}
		}
		
		void new_turn()
		{
			if((turn == players.size()) && (round < 4))
			{
				round++;
				turn = 1;
											
				//send message to next player
				nlohmann::json::object_t object_value = {{"action",NEW_TURN},{"round",round},{"turn",turn},{"pot",pot},{"minBet",minBet},{"uid",players.at(turn-1)->getUID()}}; // configure json object with parameters
				nlohmann::json j_object_value(object_value); // add it to the json
				
				std::stringstream ss;
				ss << j_object_value; // move the json into a string format
				std::string js = ss.str(); // put it into a string
											
				chat_message msg;
				msg.body_length(std::strlen(js.c_str()));// encode the string
				std::memcpy(msg.body(),js.c_str(), msg.body_length());
				msg.encode_header();
																
				room_.deliver(msg); // delivers message to everyone connected
			}
			else if(round < 4)
			{
				turn++;
				nlohmann::json::object_t object_value = {{"action",NEW_TURN},{"round",round},{"turn",turn},{"pot",pot},{"minBet",minBet},{"uid",players.at(turn-1)->getUID()}}; // configure json object with parameters
				nlohmann::json j_object_value(object_value); // add it to the json
				
				std::stringstream ss;
				ss << j_object_value; // move the json into a string format
				std::string js = ss.str(); // put it into a string
											
				chat_message msg;
				msg.body_length(std::strlen(js.c_str()));// encode the string
				std::memcpy(msg.body(),js.c_str(), msg.body_length());
				msg.encode_header();
															
				room_.deliver(msg); // delivers message to everyone connected
			}
			else
			{
				game_over();
			}
		}

  		void do_read_body()
  		{
    			auto self(shared_from_this());
    			asio::async_read(socket_,asio::buffer(read_msg_.body(), read_msg_.body_length()),[this, self](std::error_code ec, std::size_t /*length*/)
        		{
          			if (!ec)
          			{
            				//read action
							std::stringstream ss;
							ss.write(read_msg_.body(),read_msg_.body_length()); //puts the message into string format
							
							nlohmann::json jstring; // create a json object to parse the string
							ss >> jstring; // put the string into the json object
							ss.str(""); // clears the string stream
							
							int action = jstring["action"];
							switch(action)
							{
								case RAISE:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && ((round == 0) || (round == 1) || (round == 3)))
									{
										Player* p = players.at(turn-1);
										p->addAmountBet(jstring["value"]);
										minBet = p->getAmountBet();
										pot = jstring["pot"];
										
										new_turn();
									}
									
									break;
								}
								case CALL:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && ((round == 0) || (round == 1) || (round == 3)))
									{
										Player* p = players.at(turn-1);
										p->addAmountBet(jstring["value"]);
										pot = jstring["pot"];
										
										new_turn();
									}
									
									break;
								}
								case CHECK:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && ((round == 1) || (round == 3)))
									{
										new_turn();
									}
									
									break;
								}
								case FOLD:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0))
									{
										Player* p = players.at(turn-1);
										
										for(auto it = players.begin(); it != players.end(); it++)
										{
											Player* p1 = *it;
											
											if(p1->getUID().compare(p->getUID()))
											{
												players.erase(it);
											}
										}
										
										standbyPlayers.push_back(p);
										
										new_turn();
									}
									
									break;
								}
								case BUYIN:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && (round == 0))
									{
										Player* p = players.at(turn-1);
										p->addAmountBet(jstring["value"]);
										minBet = p->getAmountBet();
										pot = jstring["pot"];
										
										new_turn();
									}
									
									break;
								}
								case TRADE:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && (round == 2))
									{
										for(int i = 0; i < 5; i++)
										{
											int j = jstring["cards"][i];
											if(j)
											{
												players.at(turn-1)->replaceCard(i,deck->getTopCard());
											}
										}
										
										turn++;
										nlohmann::json::object_t object_value = {{"action",NEW_HAND},{"turn",turn},{"uid",players.at(turn-1)->getUID()},{"hand",players.at(turn-1)->getHand()}}; // configure json object with parameters
										nlohmann::json j_object_value(object_value); // add it to the json
										
										std::stringstream ss;
										ss << j_object_value; // move the json into a string format
										std::string js = ss.str(); // put it into a string
																	
										chat_message msg;
										msg.body_length(std::strlen(js.c_str()));// encode the string
										std::memcpy(msg.body(),js.c_str(), msg.body_length());
										msg.encode_header();
																					
										room_.deliver(msg); // delivers message to everyone connected
										
										new_turn();
									}
									
									break;
								}
								case ALLIN:
								{
									unsigned int turnID = jstring["turnID"];
									std::string uid = jstring["uid"];
									
									if((turnID == turn) && (uid.compare(players.at(turn-1)->getUID()) == 0) && ((round == 0) || (round == 1) || (round == 3)))
									{
										Player* p = players.at(turn-1);
										p->addAmountBet(jstring["value"]);
										minBet = p->getAmountBet();
										pot = jstring["pot"];
										
										new_turn();
									}
									
									break;
								}
								case NEW_PLAYER:
								{
									//create new player object
									Player* new_player = new Player(jstring["name"],jstring["age"]);
									
									// create a string version of the uid
									ss << uids;
									new_player->setUID(ss.str()); //set the player's uid
									uids++; // increment the uids
									ss.str(""); // clear the stream
									
									// add the player to the vector
									if((players.size() < 5) && (!game_on)) // if the current player vector is full, add to spectator
									{
										players.push_back(new_player);
									}
									else
									{
										spectators.push_back(new_player);
									}
									
									nlohmann::json::object_t object_value = {{"action",UID},{"age",new_player->getAge()},{"name",new_player->getName()},{"uid",new_player->getUID()}}; // configure json object with parameters
									nlohmann::json j_object_value(object_value); // add it to the json
									
									ss << j_object_value; // move the json into a string format
									std::string js = ss.str(); // put it into a string
	
									chat_message msg;
									msg.body_length(std::strlen(js.c_str()));// encode the string
									std::memcpy(msg.body(),js.c_str(), msg.body_length());
									msg.encode_header();
									
									room_.deliver(msg); // delivers message to everyone connected
									
									//check to see if 2 or more players are connected
									if((players.size() >= 2) && !game_on)
									{
										game_start();
									}
									
									break;
								}
								case DEL_PLAYER:
								{
									
									//go through all the players in the active players vector
									
									for(auto it = players.begin(); it != players.end(); it++)
									{
										Player* p = *it;
										
										if(p->getUID().compare(jstring["uid"]))
										{
											players.erase(it);
											delete p;
										}
									}
									
									//go through all the players in the standby vector
									
									for(auto it = standbyPlayers.begin(); it != standbyPlayers.end(); it++)
									{
										Player* p = *it;
										
										if(p->getUID().compare(jstring["uid"]))
										{
											players.erase(it);
											delete p;
										}
									}
									
									//go through all the players in the spectator vector
									
									for(auto it = spectators.begin(); it != spectators.end(); it++)
									{
										Player* p = *it;
										
										if(p->getUID().compare(jstring["uid"]))
										{
											players.erase(it);
											delete p;
										}
									}
									
									break;
								}
							}
							//room_.deliver(read_msg_); // delivers message to everyone connected
            				do_read_header();
          			}
          			else
          			{
            				room_.leave(shared_from_this());
          			}
        		});
  		}

  		void do_write()
  		{
    			auto self(shared_from_this());
    			asio::async_write(socket_,asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),[this, self](std::error_code ec, std::size_t /*length*/)
        		{
          			if (!ec)
          			{
            				write_msgs_.pop_front();
            				if (!write_msgs_.empty())
            				{
              					do_write();
            				}
          			}
          			else
          			{
            				room_.leave(shared_from_this());
          			}
        		});
  		}
		
		bool game_on;
		unsigned int turn;
		int pot;
		int minBet;
		int round;
		std::vector<Player*> players;
		std::vector<Player*> standbyPlayers;
		std::vector<Player*> spectators;
		Deck* deck;
  		tcp::socket socket_;
  		chat_room& room_;
  		chat_message read_msg_;
  		chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server
{
	public:
  		chat_server(asio::io_context& io_context,const tcp::endpoint& endpoint) : acceptor_(io_context, endpoint)
  		{
    			do_accept();
  		}

	private:
 		void do_accept()
  		{
    			acceptor_.async_accept([this](std::error_code ec, tcp::socket socket)
        		{
          			if (!ec)
          			{
            				std::make_shared<chat_session>(std::move(socket), room_)->start();
          			}

          			do_accept();
        		});
  		}

  		tcp::acceptor acceptor_;
  		chat_room room_;
};

//----------------------------------------------------------------------
int chat_session::uids = 1000;
//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  	try
  	{
    		if (argc < 2)
    		{
      			std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      			return 1;
    		}

    		asio::io_context io_context;

    		std::list<chat_server> servers;
    		for (int i = 1; i < argc; ++i)
    		{
      			tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      			servers.emplace_back(io_context, endpoint);
    		}

    		io_context.run();
  	}
  	catch (std::exception& e)
  	{
    		std::cerr << "Exception: " << e.what() << "\n";
  	}

  	return 0;
}
