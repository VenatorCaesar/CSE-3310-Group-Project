#include <gtkmm.h>
#include "chat_client.hpp"
#include "json.hpp"

#define CONT_PLAYER 8

class GameoverWindow : public Gtk::Window
{
	public:
		GameoverWindow(int result, chat_client* C, Player* p);
		~GameoverWindow();
		void on_button_continue_clicked();
		void on_button_quit_clicked();
	protected:
		Gtk::Box* MainBox;
		Gtk::Label* resultLabel;
		Gtk::Label* playLabel;
		Gtk::Button* continueButton;
		Gtk::Button* quitButton;
		chat_client* c;
		Player* me;
};
