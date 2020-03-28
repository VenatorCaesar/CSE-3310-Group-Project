#include <sstream>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <gtkmm.h>
#include <sstream>
#include "InfoWindow.hpp"
#include "GameWindow.hpp"
#include "asio.hpp"
#include "chat_message.hpp"
#include "json.hpp"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

nlohmann::json* createInfoWindow(int argc, char** argv)
{
	auto app = Gtk::Application::create(argc,argv,"");
	
	int* age = new int;
	std::string* name = new std::string;
	
	InfoWindow myInfoWindow(name,age);
	
	app->run(myInfoWindow);
	
	nlohmann::json::object_t object_value = {{"age",*age},{"name",*name}};//
	nlohmann::json* j_object_value = new nlohmann::json(object_value);//
	nlohmann::json myData = *j_object_value;
	
	if(myData["age"] < 21)
	{
		std::cout << "You are not old enough to be playing this\n";
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

		//Create the io context for the client
		asio::io_context io_context;

		//Connect to the server
		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(argv[1], argv[2]);
		chat_client* c = new chat_client(io_context, endpoints);
		assert(c);
		std::thread t([&io_context](){io_context.run();});
		
		//Get ready to run new GUI window
		int noArgs = argc-argc;
		auto app = Gtk::Application::create(noArgs,argv,"");
		
		//Create a player for myself
		Player* me = new Player(myData["name"],myData["age"]);
		
		//Get rid of old json variables
		delete creationJSON;
		myData = NULL;
		
		//Create the new game window
		GameWindow myGameWindow(me,c);
		
		//Run the game window
		app->run(myGameWindow);

		c->close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
