#include <sstream>
#include <deque>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <gtkmm.h>
#include <glibmm/iochannel.h>
#include <sstream>
#include <X11/Xlib.h>
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

// Edited the constructor to set the Player reference
chat_client::chat_client(asio::io_context& io_context,const tcp::resolver::results_type& endpoints,Player* m): io_context_(io_context),socket_(io_context)
{
	me = m;
	do_connect(endpoints);
}

//given write function
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

//given close function
void chat_client::close()
{
    asio::post(io_context_, [this]() { socket_.close(); });
}

//give connect function
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

//given read header function
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

// When the server sends a message to each client in the chat room, this is where the client receives the message
void chat_client::do_read_body()
{
	asio::async_read(socket_,asio::buffer(read_msg_.body(), read_msg_.body_length()),[this](std::error_code ec, std::size_t /*length*/) // CSE3310 This is where the message is received from the server
	{
		if (!ec)
		{
			// Convert chat_message to string
			std::stringstream ss;
			ss.write(read_msg_.body(), read_msg_.body_length());//
			
			// Parse string into json object
			nlohmann::json jstring;
			ss >> jstring;
			ss.str(""); // clears the string stream
			
			// Grab the action that the server is telling us to do
			int action = jstring["action"];
			
			switch(action)
			{
				// If the server is assigning a UID
				case UID:
				{
					// Grab name of the Player
					std::string name = jstring["name"];
					
					// If the Player's name and age matches, assign the UID to me
					if((name.compare(me->getName()) == 0) && (jstring["age"] == me->getAge()) && (me->getUID().compare("") == 0))
					{

						me->setUID(jstring["uid"]);
						me->spec = jstring["spec"];
					}
					
					break;
				}
				// If the server is sending hands to players
				case HANDS:
				{
					gdk_threads_enter(); // locks the thread
					while(myGameWindow == NULL) // Using thread racing magic to tell thread to wait for the window to exist
					{}
					myGameWindow->game_over = 0;
					me->folded = 0;
					me->all_in = 0;
					std::string uID = jstring["uid"]; // Grab the UID for the Player's hand
					if(uID.compare(me->getUID()) == 0) // if the UID matches our UID
					{
						// Set our turn ID and set the game round
						me->setTurnID(jstring["turnID"]);
						me->round = jstring["round"];
						
						// Initialize a temp pointer to store our hand in
						char new_hand[HAND_SIZE];
						std::string tempS = jstring["hand"]; //Grab the hand in string format
						
						for(int i = 0; i < HAND_SIZE; i++)
						{
							new_hand[i] = (char) tempS.at(i); // Grab the character at index i and set it to the hand
						}
						
						me->setHand(new_hand); // Set the player's hand to what we received
						myGameWindow->addPlayer(me); // Add the player to the window
						// update gui for turn notifier
						me->turn = jstring["turn"];
						me->round = jstring["round"];
						
						myGameWindow->updateTurn();
						myGameWindow->updateRound();
					}
					else // If UID doesn't match ours
					{
						std::string name = jstring["name"]; // Grab the Player's name
						int age = jstring["age"]; // Grab their age
						Player* p = new Player(name,age); // Create a new Player with the name and age received
						
						// Add new player to game window
						if(myGameWindow == NULL)
						{
							std::cout << "NULL" << std::endl;
						}
						myGameWindow->addPlayer(p);
						
						// Add player to a vector of players, that way we can delete the memory later
						listOfOpponents.push_back(p);
						//update gui for turn notifier
						myGameWindow->updateTurn();
						myGameWindow->updateRound();
					}
					
					gdk_threads_leave(); // unlocks the thread
					
					break;
				}
				// If the game is over
				case GAME_OVER:
				{
					myGameWindow->game_over = 1;
					std::cout << "Game over\n";
					gdk_threads_enter(); // locks the thread
					int tie = jstring["tie"]; // Grab a c-style bool to check for tie
					if(tie)
					{
						std::stringstream ss; // set up a stringstream to format strings to check for winner
						int i;
						int result = 0; // if we were a part of the tie
						int numWinners = 0;
						
						for(i = 1; i < 5; i++)
						{
							ss << "name" << i; // create the key
							std::string t_uid = jstring[ss.str()]; // grab the uid at that key
							if(t_uid.compare(me->getUID()) == 0) // if it is our uid
							{
								result = 1; // we were a part of the tie
							}
							
							// Check how many people won
							if(t_uid.compare("") != 0)
							{
								numWinners++;
							}
						}
						
						me->setPot(0);
						
						if(result)
						{
							std::cout << "I won\n";
							int val = jstring["pot"];
							val /= numWinners;// How much money we earned is split between numWinners amount of people
							me->setBalance(me->getBalance() + val);
						}
						
						// Get ready to create a GameoverWindow
						int noArgs = 0;
						char** args;
						auto app = Gtk::Application::create(noArgs,args,"");
						GameoverWindow* goWindow = new GameoverWindow(result,this,me); // Create the window
						app->run(*goWindow); // Run the window
						
						delete goWindow; // Delete the window after it is hidden
					}
					else
					{
						std::string t_uid = jstring["uid"]; // if it wasn't a tie, then there is only one uid
						int result = 0;
						if(t_uid.compare(me->getUID()) == 0) // check to see if it is our uid
						{
							result = 1; // if so, set it to true
						}
						
						me->setPot(0);
						
						if(result)
						{
							std::cout << "I won\n";
							int val = jstring["pot"]; // How much money we earned
							me->setBalance(me->getBalance() + val); // Add it to our balance
							std::cout << "My Bal is: " << me->getBalance() << std::endl;
						}
						
						// Get ready to create a GameoverWindow
						int noArgs = 0;
						char** args;
						auto app = Gtk::Application::create(noArgs,args,"");
						GameoverWindow* goWindow = new GameoverWindow(result,this,me); // Create the window
						app->run(*goWindow); // Run the window
						
						delete goWindow; // Delete the window after it is hidden
					}
					
					myGameWindow->removePlayers(); // Remove all players from gui
					
					me->round = jstring["round"]; // grab round counter
					me->setMinBetNeeded(0); // update the minimum bet required
					me->turn = -1;
					myGameWindow->updateTurn(); // update turn for GUI
					myGameWindow->updateRound(); // update round for GUI
					myGameWindow->updatePot(); // update pot for gui
					myGameWindow->updateBal(); // update bal for gui
					
					// Delete all the player pointers stored in listOfOpponents
					for(auto p : listOfOpponents)
					{
						delete p;
					}
					
					listOfOpponents.erase(listOfOpponents.begin(),listOfOpponents.end()); // Empty the vector
					
					gdk_threads_leave(); // unlocks thread
					
					break;
				}
				// If new turn is to happen
				case NEW_TURN:
				{
					gdk_threads_enter(); // locks thread
					me->turn = jstring["turn"]; // grab turn counter
					me->round = jstring["round"]; // grab round counter
					int new_pot = jstring["pot"]; // grab pot counter
					int new_min_bet = jstring["minBet"]; // grab the minimum bet required
					me->setPot(new_pot); // update the pot counter
					me->setMinBetNeeded(new_min_bet); // update the minimum bet required
					myGameWindow->updateTurn(); // update turn for GUI
					myGameWindow->updateRound(); // update round for GUI
					myGameWindow->updatePot(); // update pot for gui
					gdk_threads_leave(); // unlocks thread
					
					break;
				}
				// If a player asked for a new hand and the server accepted
				case NEW_HAND:
				{
					gdk_threads_enter(); // locks the thread
					std::string uID = jstring["uid"]; // grabs the uid
					if(uID.compare(me->getUID()) == 0) // if it is our uid
					{
						char new_hand[HAND_SIZE]; // temp hand to store it in
						
						std::string tempS = jstring["hand"];
						
						for(int i = 0; i < HAND_SIZE; i++)
						{
							new_hand[i] = (char) tempS.at(i); // set card at i to whichever char it is at index i
						}
						
						me->setHand(new_hand); // set the new hand
						myGameWindow->changeCards(); // update the window
					}
					gdk_threads_leave(); // unlocks the thread
					break;
				}
				// If a spectator joins // Still a bit finicky
				case SPEC:
				{
					gdk_threads_enter(); // locks the thread
					// Grabs the name of the new spectator
					std::string name = jstring["name"];
					
					myGameWindow->updateSpecList(name); // updates the label to have the spectator's names
					gdk_threads_leave(); // unlocks the thread
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

// given do_write function
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

// Builds the infowindow and will return the name and age entered by the player
nlohmann::json* createInfoWindow(int argc, char** argv)//
{
	// Create an application to run the InfoWindow
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
		exit(0);
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
		
		XInitThreads();
		
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
		
		//Create the new game window
		myGameWindow = new GameWindow(me,c);
		myGameWindow->game_over = 0;
		
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
