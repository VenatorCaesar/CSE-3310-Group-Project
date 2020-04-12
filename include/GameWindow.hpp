// Hunter Redhead
// 1001588696

#include <gtkmm.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
//#include "Player.hpp"
#include "chat_client.hpp"
#include "json.hpp"

#define RAISE 0
#define CALL 1
#define CHECK 2
#define FOLD 3
#define BUYIN 4
#define TRADE 5
#define ALLIN 6

class GameWindow : public Gtk::Window
{
	public:
		GameWindow(Player* p,chat_client* C);
		~GameWindow();
		void on_button_raise_clicked();
		void on_button_call_clicked();
		void on_button_check_clicked();
		void on_button_fold_clicked();
		void on_button_buy_in_clicked();
		void on_button_trade_clicked();
		void on_button_all_in_clicked();
        void on_button_quit_clicked();
		void addPlayer(Player* player);
		void removePlayers();
		void changeCards(Player* player);
		void on_button_card_1_clicked();
		void on_button_card_2_clicked();
		void on_button_card_3_clicked();
		void on_button_card_4_clicked();
		void on_button_card_5_clicked();
	protected:
		Gtk::Box* MainHBox; //
		Gtk::Box* PlayersVBox; //
		Gtk::Box* ValuesVBox;//
		Gtk::Label* balance;//
		Gtk::Label* pot;//
		Gtk::Label* round;//
		Gtk::Box* ButtonsVBox;//
		Gtk::Box* RaiseHBox;//
		Gtk::Button* raiseButton;//
		Gtk::Entry* raiseField;//
		Gtk::Button* callButton;//
		Gtk::Button* checkButton;//
		Gtk::Button* foldButton;//
		Gtk::Button* buy_inButton;//
		Gtk::Button* tradeButton;//
		Gtk::Button* all_inButton;//
		Gtk::Box* SideVBox;//
		Gtk::Label* SpectatorList;//
		Gtk::Button* quitButton;//
		std::vector<Gtk::Box*> PlayerBoxes;//
		std::vector<Gtk::Button*> playerCards;
		Player* me;//
		Gtk::Image* cardImages[4][13];//
		Gtk::Image* backOfCard;//
		chat_client* c;
};
