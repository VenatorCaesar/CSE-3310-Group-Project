#include <map>
#include "Deck.hpp"
#include "Player.hpp"

#define BOOST_TEST_MODULE cardtest
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (cards_tests)

BOOST_AUTO_TEST_CASE (total_cards)
{
   // check if there are 52 cards after generating a deck
   Deck  D;
   int count = 0;

   while (D.getTopCard())
   {
      count++;
   }
   BOOST_CHECK ( count == DECK_SIZE );
}

BOOST_AUTO_TEST_CASE (number_of_cards_in_suit)
{
   // want to make sure that there are total 13 cards for each suit

   Deck D;
   std::map<int,int> cards;
   int i;

   for (i = 0; i < DECK_SIZE; i++)
   {
      char c = D.getTopCard();
      cards[int(c)%16]++;     //changing hexadecial characters into decimal
   }
   BOOST_CHECK_EQUAL(cards.size(), 13);
}

BOOST_AUTO_TEST_CASE(compare_shuffle_and_new_deck)
{
   // want to make sure that the shuffle deck function works correctly.
   // Since shuffle is created randomly, it is possible that the shuffled deck
   // and the newly created deck be same but it's higly unlikely. Hence, I
   // performed this test

   Deck D1;
   Deck D2;
   D2.shuffle();
   int i;
   int count = 0;

   for(i = 0; i < DECK_SIZE; i++)
   {
      if (D1.getTopCard() != D2.getTopCard())
      {
         count++;
      }
   }
   BOOST_CHECK(count > 0);
}

BOOST_AUTO_TEST_CASE(initial_balance)
{
   // want to make sure that the initial balance of a player is 1000
   Player P("Bipul", 21);
   BOOST_CHECK(P.getBalance() == 1000);

}

 BOOST_AUTO_TEST_CASE(sort_Hand_function)
{
   // want to make sure that the sort hand function is working properly
   Deck D;
   Player P("Bipul", 21);
   D.shuffle();
   int i;
   int count = 0;
   char cards[HAND_SIZE];

   for(i = 0; i < HAND_SIZE; i++)
   {
      cards[i] = D.getTopCard();
   }
   P.setHand(cards);
   P.sortHand();
   
   for (i = 1; i < HAND_SIZE; i++)
   {

     if ((P.getHand()[i-1] & 0x0F) >  (P.getHand()[i] & 0x0F))
     {
      count++;
     }

   }
   BOOST_CHECK(count == 0);

}

BOOST_AUTO_TEST_SUITE_END( )
