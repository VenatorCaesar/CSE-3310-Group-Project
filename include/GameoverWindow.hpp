#include <gtkmm.h>

class GameoverWindow : public Gtk::Window
{
	public:
		GameoverWindow(int result);
		~GameoverWindow();
		void on_button_continue_clicked();
		void on_button_quit_clicked();
	protected:
		Gtk::Box* MainBox;
		Gtk::Label* resultLabel;
		Gtk::Label* playLabel;
		Gtk::Button* continueButton;
		Gtk::Button* quitButton;
};
