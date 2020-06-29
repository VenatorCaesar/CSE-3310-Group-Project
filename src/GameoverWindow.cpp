#include "GameoverWindow.hpp"

/*
	result: 0 for loss
		1 for won
*/
GameoverWindow::GameoverWindow(int result, chat_client* C, Player* p)
{
	c = C;
	me = p;
	set_title("Poker++ Game Over Message");
	set_default_size(125, 125);

	std::string rLabel = "";
	if(result == 0)
	{
		rLabel = "GAME OVER! You loss!\n";
	}
	else if(result == 1)
	{
		rLabel = "GAME OVER! You won!\n";
	}
	else
	{
		rLabel = "GAME ERROR: GameoverWindow result was not a correct value...\n";
	}

	//rLabel.append("\nDo you want to continue playing?");

	MainBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
	add(*MainBox);

	resultLabel = Gtk::manage(new Gtk::Label{rLabel});
	MainBox->add(*resultLabel);

	playLabel = Gtk::manage(new Gtk::Label{"Do you want to continue playing?\n"});
	MainBox->add(*playLabel);
	
	continueButton = Gtk::manage(new Gtk::Button("Continue"));
	MainBox->add(*continueButton);
	continueButton->signal_clicked().connect(sigc::mem_fun(*this, &GameoverWindow::on_button_continue_clicked));

	quitButton = Gtk::manage(new Gtk::Button("Quit"));
	MainBox->add(*quitButton);
	quitButton->signal_clicked().connect(sigc::mem_fun(*this, &GameoverWindow::on_button_quit_clicked));
	
	MainBox->show_all();
}

GameoverWindow::~GameoverWindow()
{
	delete MainBox;
}

void GameoverWindow::on_button_continue_clicked()
{
	int action = CONT_PLAYER;
		
	nlohmann::json::object_t object_value = {{"uid",me->getUID()},{"action",action},{"name",me->getName()},{"age",me->getAge()}};//
	nlohmann::json j_object_value(object_value);
		
	std::stringstream ss;
	chat_message msg;
	ss << j_object_value;
	std::string js = ss.str();
		
	msg.body_length(std::strlen(js.c_str()));//
	std::memcpy(msg.body(),js.c_str(), msg.body_length());//
	msg.encode_header();
	c->write(msg);
	
	hide();
}

void GameoverWindow::on_button_quit_clicked()
{		
	hide();
	exit(0);
}
