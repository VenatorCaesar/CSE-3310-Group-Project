#include "GameoverWindow.hpp"

/*
	result: 0 for loss
			1 for won
*/
GameoverWindow::GameoverWindow(int result)
{
	set_title("Poker++");
	set_default_size(125, 125);

	std::string rLabel = " ";
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
	
	show_all_children();
}

GameoverWindow::~GameoverWindow()
{

}

void GameoverWindow::on_button_continue_clicked()
{
	// Do something
}

void GameoverWindow::on_button_quit_clicked()
{
	// Do something
}
