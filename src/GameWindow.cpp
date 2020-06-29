
#include "GameWindow.hpp"

GameWindow::GameWindow(Player* p,chat_client* C)
{
	me = p;
	c = C;
	for(int i = 0; i < 5; i++)
	{
		idx[i] = 0;
	}
	
	//Set title and default size
	set_title("Poker++");
	set_default_size(1000,1000);
	
	//Creates horizontal box to store other widgets
	MainHBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
	add(*MainHBox);
	
	//Creates the player VBox, but does not add any players
	PlayersVBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
	MainHBox->add(*PlayersVBox);
	
	//Creates VBox for labels for balance and pot
	ValuesVBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
	MainHBox->add(*ValuesVBox);
	
	std::stringstream ss;
	
	//Adds the labels for values
	ss << "Balance: $" << me->getBalance();
	balance = Gtk::manage(new Gtk::Label{ss.str()});
	ValuesVBox->add(*balance);
	ss.str("");
	
	ss << "Pot Value: $" << me->getPot();
	pot = Gtk::manage(new Gtk::Label{ss.str()});
	ValuesVBox->add(*pot);
	ss.str("");
	
	roundL = Gtk::manage(new Gtk::Label{"Round 0"});
	ValuesVBox->add(*roundL);
	
	turnL = Gtk::manage(new Gtk::Label{"Turn 1"});
	ValuesVBox->add(*turnL);
	
	//Adds VBox for buttons
	ButtonsVBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
	MainHBox->add(*ButtonsVBox);
	
	//Adds Raise Button and Field
	RaiseHBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
	ButtonsVBox->add(*RaiseHBox);
	
	raiseButton = Gtk::manage(new Gtk::Button("Raise"));
	RaiseHBox->add(*raiseButton);
	raiseButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_raise_clicked));
	
	raiseField = Gtk::manage(new Gtk::Entry());
	raiseField->set_placeholder_text("Enter Amount to Bet");
	RaiseHBox->add(*raiseField);
	raiseField->signal_activate().connect(sigc::mem_fun(*this, &GameWindow::on_button_raise_clicked));
	
	//Adds the other buttons
	callButton = Gtk::manage(new Gtk::Button("Call"));
	ButtonsVBox->add(*callButton);
	callButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_call_clicked));
	
	checkButton = Gtk::manage(new Gtk::Button("Check"));
	ButtonsVBox->add(*checkButton);
	checkButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_check_clicked));
	
	foldButton = Gtk::manage(new Gtk::Button("Fold"));
	ButtonsVBox->add(*foldButton);
	foldButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_fold_clicked));
	
	buy_inButton = Gtk::manage(new Gtk::Button("Buy-In"));
	ButtonsVBox->add(*buy_inButton);
	buy_inButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_buy_in_clicked));
	
	tradeButton = Gtk::manage(new Gtk::Button("Trade"));
	ButtonsVBox->add(*tradeButton);
	tradeButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_trade_clicked));
	
	all_inButton = Gtk::manage(new Gtk::Button("All-In"));
	ButtonsVBox->add(*all_inButton);
	all_inButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_all_in_clicked));
	
	continueButton = Gtk::manage(new Gtk::Button("Continue Playing"));
	ButtonsVBox->add(*continueButton);
	continueButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_continue_clicked));
	
	//Adds the list of spectators and quit button
	SideVBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0));
	MainHBox->add(*SideVBox);
	
	SpectatorList = Gtk::manage(new Gtk::Label{"Spectators:\n"});
	SideVBox->add(*SpectatorList);
	
	quitButton = Gtk::manage(new Gtk::Button("Quit"));
	SideVBox->add(*quitButton);
	quitButton->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_quit_clicked));
	
	//Adding all the images to the matrix
	
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 0x0D; j++)
		{
			ss << "JPEG/rsz_" << i+1 << std::hex << std::noshowbase << j+1 << ".jpg";
			cardImages[i][j] = new Gtk::Image(ss.str());
			ss.str("");
		}
	}
	
	MainHBox->show_all();
}

GameWindow::~GameWindow()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j =0; j < 0x0D; j++)
		{
			delete cardImages[i][j];
		}
	}
	
	delete MainHBox;
}

void GameWindow::on_button_raise_clicked()//
{
	if((me->turn == me->getTurnID()) && (me->round != 2))
	{
		//Grab text
		std::string entered = raiseField->get_text();
		//Convert to int
		int val;
		try
		{
			val = std::stoi(entered);
		}
		catch(const std::invalid_argument& ia)
		{
			std::cerr << "Enter an integer value please" << std::endl;
			return;
		}
		
		//If entered exceeds funds, go all in
		if(val > me->getBalance())
		{
			//set status to all in
			val = me->getBalance();
			int action = ALLIN;
			
			me->setPot(me->getPot() + val);
			
			//Update balance
			std::stringstream ss;
			ss << "Balance: $" << me->getBalance();
			balance->set_text(ss.str());
			ss.str("");
		
			//Needs to update pot value and update the label for the pot value
			ss << "Pot Value: $" << me->getPot();
			pot->set_text(ss.str());
			ss.str("");
			
			chat_message msg;
			nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
			nlohmann::json j_object_value(object_value);
		
			ss << j_object_value;
			std::string js = ss.str();
		
			msg.body_length(std::strlen(js.c_str()));//
			std::memcpy(msg.body(),js.c_str(), msg.body_length());//
			msg.encode_header();
			c->write(msg);
			
			return;
		}
		
		//Remove value raised from player's balance
		me->setBalance(me->getBalance() - val);
		me->addAmountBet(val);
		raiseField->set_placeholder_text("Enter Amount to Bet");
		
		//Update balance
		std::stringstream ss;
		ss << "Balance: $" << me->getBalance();
		balance->set_text(ss.str());
		ss.str("");
		
		me->setPot(me->getPot() + val);
		
		//Needs to update pot value and then update the label for the pot value
		ss << "Pot Value: $" << me->getPot();
		pot->set_text(ss.str());
		ss.str("");
		
		//Construct client message to server
		int action = RAISE;
		
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
		nlohmann::json j_object_value(object_value);
		
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
	
	return;
}

void GameWindow::on_button_call_clicked()//
{
	if((me->turn == me->getTurnID()) && (me->round != 2))
	{
		//Get value needed to bet
		int val = me->getMinBetNeeded() - me->getAmountBet();
		
		if(val > me->getBalance())
		{
			//set status to all in
			val = me->getBalance();
			int action = ALLIN;
			
			me->setBalance(me->getBalance() - val);
			me->addAmountBet(val);
			
			me->setPot(me->getPot() + val);
			
			//Update balance
			std::stringstream ss;
			ss << "Balance: $" << me->getBalance();
			balance->set_text(ss.str());
			ss.str("");
		
			//Needs to update pot value and update the label for the pot value
			ss << "Pot Value: $" << me->getPot();
			pot->set_text(ss.str());
			ss.str("");
		
			nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
			nlohmann::json j_object_value(object_value);
			
			chat_message msg;
			ss << j_object_value;
			std::string js = ss.str();
		
			msg.body_length(std::strlen(js.c_str()));//
			std::memcpy(msg.body(),js.c_str(), msg.body_length());//
			msg.encode_header();
			c->write(msg);
			
			return;
		}
		
		if(me->getAmountBet() < me->getMinBetNeeded())
		{
			//Remove value raised from player's balance
			me->setBalance(me->getBalance() - val);
			me->setPot(me->getPot() + val);
			me->addAmountBet(val);
			
			//Update balance
			std::stringstream ss;
			ss << "Balance: $" << me->getBalance();
			balance->set_text(ss.str());
			ss.str("");
			
			//Needs to update pot value and update the label for the pot value
			ss << "Pot Value: $" << me->getPot();
			pot->set_text(ss.str());
			ss.str("");
			
			//Construct client message to server
			int action = CALL;
			
			nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
			nlohmann::json j_object_value(object_value);
			
			chat_message msg;
			ss << j_object_value;
			std::string js = ss.str();
			
			msg.body_length(std::strlen(js.c_str()));//
			std::memcpy(msg.body(),js.c_str(), msg.body_length());//
			msg.encode_header();
			c->write(msg);
		}
	}
	return;
}

void GameWindow::on_button_check_clicked()//
{
	if(me->turn == me->getTurnID())
	{
		// Check to see I meet the bet requirements
		if(me->getAmountBet() < me->getMinBetNeeded())
		{
			std::cout << me->getAmountBet() << "\n" << me->getMinBetNeeded() << std::endl;
			return; // if i don't act like the user didn't press it
		}
		
		int action = CHECK; // set the action
		
		// put it into json format
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action}};//
		nlohmann::json j_object_value(object_value);
		
		std::stringstream ss;
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		// construct the message to the server
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
	
	return;
}

void GameWindow::on_button_fold_clicked()//
{
	if(me->turn == me->getTurnID())
	{
		for(auto card : playerCards)
		{
			Gtk::Image* cardImage = Gtk::manage(new Gtk::Image("JPEG/back_of_card.jpg")); // Get the back of card image
			
			card->set_image(*cardImage); // set the button's image to back of card
		}
		me->folded = 1;
		
		int action = FOLD;
		
		// Begin constructing the message to the server
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",0},{"pot",me->getPot()}};//
		nlohmann::json j_object_value(object_value);
		
		std::stringstream ss;
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
	
	return;
}

void GameWindow::on_button_buy_in_clicked()//
{
	if((me->turn == me->getTurnID()) && (me->round == 0))
	{
		int val = 10; //buy in amount that I decided on, can change if you want it to
		if(val > me->getBalance())
		{
			val = me->getBalance();
			int action = ALLIN;
			
			me->setBalance(me->getBalance() - val);
			me->addAmountBet(val);
			me->setPot(me->getPot() + val);
			
			//Update balance
			std::stringstream ss;
			ss << "Balance: $" << me->getBalance();
			balance->set_text(ss.str());
			ss.str("");
		
			//Needs to update pot value and update the label for the pot value
			ss << "Pot Value: $" << me->getPot();
			pot->set_text(ss.str());
			ss.str("");
		
			nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
			nlohmann::json j_object_value(object_value);
			
			chat_message msg;
			ss << j_object_value;
			std::string js = ss.str();
		
			msg.body_length(std::strlen(js.c_str()));//
			std::memcpy(msg.body(),js.c_str(), msg.body_length());//
			msg.encode_header();
			c->write(msg);
			
			return;
		}
		
		//Remove value raised from player's balance
		me->setBalance(me->getBalance() - val);
		me->addAmountBet(val);
		me->setPot(me->getPot() + val);
		
		//Update balance
		std::stringstream ss;
		ss << "Balance: $" << me->getBalance();
		balance->set_text(ss.str());
		ss.str("");
		
		//Needs to update pot value and update the label for the pot value
		ss << "Pot Value: $" << me->getPot();
		pot->set_text(ss.str());
		ss.str("");
		
		int action = BUYIN;
		
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
		nlohmann::json j_object_value(object_value);
			
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
	
	return;
}

void GameWindow::on_button_trade_clicked()
{
	if((me->turn == me->getTurnID()) && (me->round == 2))
	{
		int action = TRADE;
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"cards",idx}};//
		nlohmann::json j_object_value(object_value);
		
		std::stringstream ss;
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
		
		for(int i = 0; i < HAND_SIZE; i++)
		{
			idx[i] = 0;
		}
	}
	
	return;
}

void GameWindow::on_button_all_in_clicked()
{
	if((me->turn == me->getTurnID()) && (me->round != 2))
	{
		//Remove value raised from player's balance
		int val = me->getBalance();
		me->setBalance(0);
		me->setPot(me->getPot() + val);
		
		//Update balance
		std::stringstream ss;
		ss << "Balance: $" << me->getBalance();
		balance->set_text(ss.str());
		ss.str("");
		
		//Update pot
		ss << "Pot Value: $" << me->getPot();
		pot->set_text(ss.str());
		ss.str("");
		
		me->all_in = 1;
		
		int action = ALLIN;
		
		nlohmann::json::object_t object_value = {{"turnID",me->getTurnID()},{"uid",me->getUID()},{"action",action},{"value",val},{"pot",me->getPot()}};//
		nlohmann::json j_object_value(object_value);
			
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
	
	return;
}

void GameWindow::on_button_continue_clicked()
{
	if(game_over)
	{
		std::stringstream ss;
		
		int action = CONT_PLAYER;
		
		nlohmann::json::object_t object_value = {{"action",action},{"uid",me->getUID()},{"name",me->getName()},{"age",me->getAge()}};//
		nlohmann::json j_object_value(object_value);
			
		chat_message msg;
		ss << j_object_value;
		std::string js = ss.str();
		
		msg.body_length(std::strlen(js.c_str()));//
		std::memcpy(msg.body(),js.c_str(), msg.body_length());//
		msg.encode_header();
		c->write(msg);
	}
}

void GameWindow::on_button_quit_clicked()
{
	if((me->round == 4) || (game_over))
	{		
		hide();
	}
}

void GameWindow::addPlayer(Player* player)
{
	// Create a new box for the new player
	Gtk::Box* newPlayerBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL,0);
	PlayersVBox->add(*newPlayerBox); // THIS IS CAUSING SEGFAULT FOR SECOND PLAYER
	PlayerBoxes.push_back(newPlayerBox);
	
	//Create a labek for the player name
	Gtk::Label* playerName = Gtk::manage(new Gtk::Label{player->getName()});
	newPlayerBox->add(*playerName);
	
	//Create an hbox for the cards of the player
	Gtk::Box* playerCardsHBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL,0));
	newPlayerBox->add(*playerCardsHBox);
	
	//Create five buttons to be added to the card h box
	if(player == me) //compare the addresses of the cards
	{
		for(int i = 0; i < 5; i++)
		{
			Gtk::Image* cardImage;
			
			int suit = me->getHand()[i] & 0xF0;
			suit = suit >> 4;
			int val = me->getHand()[i] & 0x0F;
			
			cardImage = cardImages[suit-1][val-1];
			
			Gtk::Button* card = Gtk::manage(new Gtk::Button());
			playerCardsHBox->add(*card);
			card->set_image_position(Gtk::POS_LEFT);
			card->set_image(*cardImage);
			switch(i)
			{
				case 0:
				{
					card->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_card_1_clicked));
					break;
				}
				case 1:
				{
					card->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_card_2_clicked));
					break;
				}
				case 2:
				{
					card->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_card_3_clicked));
					break;
				}
				case 3:
				{
					card->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_card_4_clicked));
					break;
				}
				case 4:
				{
					card->signal_clicked().connect(sigc::mem_fun(*this, &GameWindow::on_button_card_5_clicked));
					break;
				}
			}
			
			playerCards.push_back(card);
		}
	}
	else
	{
		for(int i = 0; i < 5; i++)
		{
			Gtk::Button* card = Gtk::manage(new Gtk::Button());
			card->set_image_position(Gtk::POS_LEFT);
			Gtk::Image* backOfCard = Gtk::manage(new Gtk::Image("JPEG/back_of_card.jpg"));
			card->set_image(*backOfCard);
			playerCardsHBox->add(*card);
		}
	}
	
	newPlayerBox->show_all();
	
	return;
}

void GameWindow::removePlayers()
{
	for(auto i : PlayerBoxes)
	{
		delete i;
	}
	
	return;
}

void GameWindow::changeCards()
{
	int i = 0;
	for(auto card : playerCards)
	{
		Gtk::Image* cardImage;
		
		std::stringstream ss;
		
		int suit = me->getHand()[i] & 0xF0;
		suit = suit >> 4;
		int val = me->getHand()[i] & 0x0F;
		
		ss << "JPEG/rsz_" << suit << std::hex << std::noshowbase << val << ".jpg";
		cardImage = Gtk::manage(new Gtk::Image(ss.str()));
		ss.str("");
		
		card->set_image(*cardImage);
		i++;
	}
	
	return;
}

void GameWindow::on_button_card_1_clicked()
{
	idx[0] = !idx[0];
	return;
}

void GameWindow::on_button_card_2_clicked()
{
	idx[1] = !idx[1];
	return;
}

void GameWindow::on_button_card_3_clicked()
{
	idx[2] = !idx[2];
	return;
}

void GameWindow::on_button_card_4_clicked()
{
	idx[3] = !idx[3];
	return;
}

void GameWindow::on_button_card_5_clicked()
{
	idx[4] = !idx[4];
	return;
}

void GameWindow::updatePot()
{
	std::stringstream ss;
	
	ss << "Pot Value: $" << me->getPot();
	pot->set_text(ss.str());
	ss.str("");
	
	return;
}

void GameWindow::updateRound()
{
	std::stringstream ss;
	
	ss << "Round: " << me->round;
	roundL->set_text(ss.str());
	ss.str("");
	
	return;
}

void GameWindow::updateTurn()
{
	std::stringstream ss;
	
	if(me->turn == me->getTurnID())
	{
		ss << "Turn: *" << me->turn;
		turnL->set_text(ss.str());
		ss.str("");
	}
	else
	{
		ss << "Turn: " << me->turn;
		turnL->set_text(ss.str());
		ss.str("");
	}
	
	return;
}

void GameWindow::updateBal()
{
	std::stringstream ss;
	
	ss << "Balance: " << me->getBalance();
	balance->set_text(ss.str());
	ss.str("");
	
	return;
}

void GameWindow::updateSpecList(std::string n)
{
	std::stringstream ss;
	ss << SpectatorList->get_text() << n << "\n";
	
	SpectatorList->set_text(ss.str());
	
	ss.str("");
	
	return;
}