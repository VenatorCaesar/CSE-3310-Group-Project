// Hunter Redhead
// 1001588696

#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>// for error handling

class InfoWindow : public Gtk::Window
{
	public:
		InfoWindow(std::string* name, int* age);
		~InfoWindow();
		void on_button_exit_clicked();
		void on_button_join_in_clicked();
	protected:
		Gtk::Box* MainHBox;
		Gtk::Button* exitButton;
		Gtk::Box* fieldsVBox;
		Gtk::Entry* nameField;
		Gtk::Entry* ageField;
		Gtk::Button* join_inButton;
		std::string* Name;
		Gtk::Image* logoImage;
		int* Age;
};