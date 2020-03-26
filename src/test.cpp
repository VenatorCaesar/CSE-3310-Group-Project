#include <iostream>
#include <string>
#include <gtkmm.h>
#include <sstream>
#include "InfoWindow.hpp"
#include "GameWindow.hpp"
#include "json.hpp"

std::string createInfoWindow(int argc, char** argv)
{
	auto app = Gtk::Application::create(argc,argv,"");
	
	int* age = new int;
	std::string* name = new std::string;
	std::string* ip = new std::string;
	std::string* port = new std::string;
	
	InfoWindow myInfoWindow(name,age);
	
	app->run(myInfoWindow);
	
	nlohmann::json::object_t object_value = {{"age",*age},{"name",*name}};//
	nlohmann::json j_object_value(object_value);//
	
	if(j_object_value["age"] < 21)
	{
		std::cout << "You are not old enough to be playing this\n";
		abort();
	}
	
	std::stringstream ss;
	
	ss << j_object_value;
	
	return ss.str();
}

int main(int argc, char** argv)
{
	try
	{
		if(argc != 3)
		{
			std::cerr << "Usage: client <host> <port>\n";
			return 1;
		}
		
		//Message to be sent to the server if
		std::string message = createInfoWindow(argc-argc,argv); //argc-argc to have to GUI ignore command line arguments
			
		//auto app = Gtk::Application::create(argc-argc,argv,"");
			
		//Player* me = new Player("Hunter",20,"1001",1,1000);
		//char hand1[HAND_SIZE] = {0x11,0x12,0x13,0x14,0x15};
		//me->setHand(hand1);
		//Player* notMe = new Player("Clayton",20,"1002",2,1000);
		//char hand2[HAND_SIZE] = {0x11,0x2A,0x3B,0x4C,0x1D};
		//notMe->setHand(hand2);
			
		//GameWindow myGameWindow(me);
		//myGameWindow.addPlayer(me);
		//myGameWindow.addPlayer(notMe);
			
		//app->run(myGameWindow);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	
	return 0;
}