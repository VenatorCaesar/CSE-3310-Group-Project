// Window to grab user info

#include "InfoWindow.hpp"

InfoWindow::InfoWindow(std::string* name, int* age)
{
	Name = name;
	Age = age;
	
	//Set title and default size
	set_title("Poker++ Info Window");
	set_default_size(100,100);
	
	//Creates the horizontal box to store other widgets
	MainHBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
	add(*MainHBox); //adds it to the window
	
	//Adds the exit button
	exitButton = Gtk::manage(new Gtk::Button("Exit"));
	MainHBox->add(*exitButton);
	exitButton->signal_clicked().connect(sigc::mem_fun(*this, &InfoWindow::on_button_exit_clicked));
	
	//Adds the VBox for the entry fields
	fieldsVBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
	MainHBox->add(*fieldsVBox);
	
	//Adds the entry fields
	nameField = Gtk::manage(new Gtk::Entry());
	nameField->set_placeholder_text("Enter Name");
	fieldsVBox->add(*nameField);
	ageField = Gtk::manage(new Gtk::Entry());
	ageField->set_placeholder_text("Enter Age");
	fieldsVBox->add(*ageField);
	
	//Adds the join button
	join_inButton = Gtk::manage(new Gtk::Button("Join In"));
	MainHBox->add(*join_inButton);
	join_inButton->signal_clicked().connect(sigc::mem_fun(*this, &InfoWindow::on_button_join_in_clicked));
	
	MainHBox->show_all();
}

InfoWindow::~InfoWindow()
{
	delete MainHBox;
}

//Code needs to be written here
void InfoWindow::on_button_join_in_clicked()
{
	//grab text from entry fields
	std::string enteredName = nameField->get_text();
	std::string entered = ageField->get_text();
	int enteredAge = std::stoi(entered);
	
	*Name = enteredName;
	*Age = enteredAge;
	
	hide();
}

void InfoWindow::on_button_exit_clicked()
{
	hide();
}