#include <sstream>
#include <deque>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <gtkmm.h>
#include <glibmm/iochannel.h>
#include <sstream>
#include "InfoWindow.hpp"
#include "GameWindow.hpp"
#include "asio.hpp"
#include "chat_message.hpp"
#include "json.hpp"
#include "GameoverWindow.hpp"

#define UID 9
#define HANDS 10
#define GAME_OVER 11
#define NEW_TURN 12
#define NEW_HAND 13
#define SPEC 14

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

GameWindow* myGameWindow;

chat_client::chat_client(asio::io_context& io_context,const tcp::resolver::results_type& endpoints,Player* m): io_context_(io_context),socket_(io_context)
{
	me = m;
	do_connect(endpoints);
}

void chat_client::write(const chat_message& msg)
{
	asio::post(io_context_,[this, msg]()
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			do_write();
		}
	});
}

void chat_client::close()
{
    asio::post(io_context_, [this]() { socket_.close(); });
}

void chat_client::do_connect(const tcp::resolver::results_type& endpoints)
{
    asio::async_connect(socket_, endpoints,[this](std::error_code ec, tcp::endpoint) // CSE3310 This is where the connection is established with the server
    {
		if (!ec)
        {
			do_read_header();
        }
	});
}

void chat_client::do_read_header()
{
	asio::async_read(socket_,asio::buffer(read_msg_.data(), chat_message::header_length),
		[this](std::error_code ec, std::size_t /*length*/)
	{
		if (!ec && read_msg_.decode_header())
		{
			do_read_body();
		}
		else
		{
			socket_.close();
		}
	});
}

void chat_client::do_read_body()
{
	asio::async_read(socket_,asio::buffer(read_msg_.body(), read_msg_.body_length()),[this](std::error_code ec, std::size_t /*length*/) // CSE3310 This is where the message is received from the server
	{
		if (!ec)
		{
			std::stringstream ss;
			ss.write(read_msg_.body(), read_msg_.body_length());//
			
			nlohmann::json jstring;
			ss >> jstring;
			ss.str(""); // clears the string stream
							
			int action = jstring["action"];
			
			switch(action)
			{
				case UID:
				{
					std::string name = jstring["name"];
					if((name.compare(me->getName()) == 0) && (jstring["age"] == me->getAge()) && (me->getUID().compare("") == 0))
					{
						me->setUID(jstring["uid"]);
					}
					
					break;
				}
				case HANDS:
				{
					std::string uID = jstring["uid"];
					if(uID.compare(me->getUID()) == 0)
					{
						//std::cout << "ME\n";
						me->setTurnID(jstring["turnID"]);
						me->round = jstring["round"];
						
						char new_hand[HAND_SIZE];
						
						for(int i = 0; i < HAND_SIZE; i++)
						{
							std::string tempS = jstring["hand"];
							//printf("%x\n",tempS.at(i));
							new_hand[i] = (char) tempS.at(i);
						}
						
						me->setHand(new_hand);
						myGameWindow->addPlayer(me);
						// update gui for turn notifier
						me->turn = jstring["turn"];
						me->round = jstring["round"];
						
						myGameWindow->updateTurn();
						myGameWindow->updateRound();
					}
					else
					{
						//std::cout << "NOT ME\n";
						std::string name = jstring["name"];
						int age = jstring["age"];
						Player* p = new Player(name,age);
						
						char new_hand[HAND_SIZE];
						
						for(int i = 0; i < HAND_SIZE; i++)
						{
							std::string tempS = jstring["hand"];
							//printf("%x\n",tempS.at(i));
							new_hand[i] = (char) tempS.at(i);
						}
						
						p->setHand(new_hand);
						myGameWindow->addPlayer(p);
						
						listOfOpponents.push_back(p);
						//update gui for turn notifier
						myGameWindow->updateTurn();
						myGameWindow->updateRound();
					}
					
					break;
				}
				case GAME_OVER:
				{
					int tie = jstring["tie"];
					if(tie)
					{
						std::stringstream ss;
						int i;
						int result = 0;
						
						for(i = 1; i < 6; i++)
						{
							ss << "name" << i;
							std::string t_uid = jstring[ss.str()];
							if(t_uid.compare(me->getUID()) == 0)
							{
								result = 1;
							}
						}
						
						int noArgs = 0;
						char** args;
						auto app = Gtk::Application::create(noArgs,args,"");
						GameoverWindow* goWindow = new GameoverWindow(result,this,me);
						app->run(*goWindow);
					}
					else
					{
						std::string t_uid = jstring["uid"];
						int result = 0;
						if(t_uid.compare(me->getUID()) == 0)
						{
							result = 1;
						}
						
						int noArgs = 0;
						char** args;
						auto app = Gtk::Application::create(noArgs,args,"");
						GameoverWindow* goWindow = new GameoverWindow(result,this,me);
						app->run(*goWindow);
					}
					
					break;
				}
				case NEW_TURN:
				{
					me->turn = jstring["turn"];
					me->round = jstring["round"];
					int new_pot = jstring["pot"];
					int new_min_bet = jstring["minBet"];
					me->setPot(new_pot);
					me->setMinBetNeeded(new_min_bet);
					myGameWindow->updateTurn();
					myGameWindow->updateRound();
					myGameWindow->updatePot();
					
					break;
				}
				case NEW_HAND:
				{
					std::string uID = jstring["uid"];
					if(uID.compare(me->getUID()) == 0)
					{
						char new_hand[HAND_SIZE];
						
						for(int i = 0; i < HAND_SIZE; i++)
						{
							std::string tempS = jstring["hand"];
							//printf("%x\n",tempS.at(i));
							new_hand[i] = (char) tempS.at(i);
						}
						
						me->setHand(new_hand);
						myGameWindow->changeCards(me);
					}
					
					break;
				}
				case SPEC:
				{
					std::stringstream ss;
					int i = 0;
					ss << "name" << i;
					
					std::stringstream l;
					
					while(jstring.contains(ss.str()))
					{
						std::string name = jstring[ss.str()];
						
						l << name << "\n";
						i++;
						ss.str("");
						ss << "name" << i;
					}
					
					myGameWindow->updateSpecList(l.str());
					l.str("");
				}
			}
			
			do_read_header();
		}
		else
		{
			socket_.close();
		}
	});
}

void chat_client::do_write()
{
	asio::async_write(socket_,asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),[this](std::error_code ec, std::size_t /*length*/)
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
			socket_.close();
		}
	});
}

nlohmann::json* createInfoWindow(int argc, char** argv)
{
	auto app = Gtk::Application::create(argc,argv,"");
	
	int* age = new int;
	std::string* name = new std::string;
	
	InfoWindow myInfoWindow(name,age);
	
	app->run(myInfoWindow);
	
	nlohmann::json::object_t object_value = {{"action",7},{"age",*age},{"name",*name}};//
	nlohmann::json* j_object_value = new nlohmann::json(object_value);//
	nlohmann::json myData = *j_object_value;
	
	if(myData["age"] < 21)
	{
		std::cerr << "You are not old enough to be playing this\n";
		abort();
	}
	
	delete name;
	delete age;
	
	return j_object_value;
}

int main(int argc, char** argv)
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>\n";
			return 1;
		}
		
		//Message to be sent to the server if of age
		nlohmann::json* creationJSON = createInfoWindow(argc-argc,argv); //argc-argc to have to GUI ignore command line arguments
		nlohmann::json myData = *creationJSON;
		
		//Create a player for myself
		Player* me = new Player(myData["name"],myData["age"]);
		
		//Create the io context for the client
		asio::io_context io_context;

		//Connect to the server
		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(argv[1], argv[2]);
		chat_client* c = new chat_client(io_context, endpoints,me);
		assert(c);
		std::thread t([&io_context](){io_context.run();});
		
		//send new player info to the server
		std::stringstream ss;
		chat_message msg;
		ss << myData;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		
		c->write(msg);
		
		//Get ready to run new GUI window
		int noArgs = argc-argc;
		auto app = Gtk::Application::create(noArgs,argv,"");
		
		//Get rid of old json variables
		delete creationJSON;
		myData = NULL;
		
		//Create the new game window
		myGameWindow = new GameWindow(me,c);
		
		//Run the game window
		app->run(*myGameWindow);

		c->close();
		t.join();
		
		delete myGameWindow;
		delete me;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
