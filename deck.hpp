//
//  deck.hpp
//  UnoPlayer
//
//  Created by Evan Scarborough on 7/6/16.
//  Copyright © 2016 VectoPlasm. All rights reserved.
//

#ifndef deck_hpp
#define deck_hpp

#include <stdio.h>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <map>
#include <iostream>

#include "ResourcePath.hpp"

using namespace std;
using namespace sf;

// This is exactly what you think.
enum COLOR{ RED, YELLOW, GREEN, BLUE, WILD };





/*
 Defines a card class. It is more or less just a data structure that holds the color and number of the card.
*/
class card{
public:
    COLOR color; // a color
    int number; // a number (skip=10, reverse=11, draw2=12)
    string cardString; // a string that is the same as the path and name of the png file to be displayed.
    
    card(COLOR colin, int num){
		// constructor
        color = colin;
        number = num;
        cardString = getCardString();
    }
	// getters. nothing should ever need to be set, so there are no setters.
    COLOR getColor(){ return color; }
    int getNumber(){ return number; }
	// takes the color and number and turns it into the file path.
    string getCardString(){
        string cc = "wild";
        if(color == RED) cc = "red";
        if(color == YELLOW) cc = "yellow";
        if(color == GREEN) cc = "green";
        if(color == BLUE) cc = "blue";
        string nn = to_string(number);
        if(number == 10) nn = "skip";
        if(number == 11) nn = "reverse";
        if(number == 12) nn = "draw";
        if(color == WILD){
            if(number == -2) nn = "draw";
            else nn = "wild";
        }
        return cc + "/" + nn;
    }
};


/*
 CardDrawer holds all of the sprites and textures to be used.
 It is designed in such a way that it will only load the sprites once they are needed.
*/
class CardDrawer{
public:
    map<string,Texture*> cardtextures; // holds all of the textues indexed by thier file path
    map<string,Sprite*> cardsprites; // holds all of the sprites indexed by thier file path
	// NOTE: maps that use strings as their index are not as fast as some other data structures.
	//       perhaps consider using an int identifier so a vector could be used.
	
	// will load a card texture and set some initial properties of the sprite.
    void registerCard(string name){
        Texture* texture = new Texture();
        Sprite* cardsprite = new Sprite();
        texture->loadFromFile(resourcePath() + "cards/" + name + ".png");
        cardsprite->setTexture(*texture);
        cardsprite->setScale(3,3);
        cardtextures.emplace(name,texture);
        cardsprites.emplace(name,cardsprite);
    }
	// gets a pointer to the sprite associated with a card.
    Sprite* getCard(card& c){
        if(cardsprites.find(c.cardString) == cardsprites.end()){
            registerCard(c.cardString);
        }
        return cardsprites.at(c.cardString);
    }
};






/*
 A player class holds information about a player like thier hand and some properties that control behavior.
*/
class player{
public:
    vector<card> hand; // player's hand
	// TODO: I should change this to use card* instead. Then cards don't get passed, but just pointers to them.
	//       that would have been much smarter.
    int wins = 0; // win counter
    int evilness; // how evil are they
    int saveWildTurns = 0; // how many draws they will hold on to wilds
	int saveWildCount = 0; // counter so that they will wait before using a wild
	// priorities, do they look for matches by color or number first? second?
	char firstPriority;
    char secondPriority;
    string name; // their name for keeping track.
	// constructor
    player(string n, int e, int sw, char first, char second){
        name = n;
        evilness = e;
        saveWildTurns = sw;
        if(first != 'n' && first != 'w') first = 'c';
        firstPriority = first;
        if(second != 'n' && second != 'w') second = 'c';
        secondPriority = second;
    }
	// pick up a card. See, this shouldn't pass a card, but rather a card*. Fix this!
    void pickUp(card c){
        hand.push_back(c);
    }
	// draws the player's name, wins, and hand to the screen.
    void draw(RenderWindow* window, Text& text, int center, double angle, double radius, CardDrawer* drawer, bool isActive){
		// position is dynamically set so that adding more players will arrange them so they don't overlap
        int xp = center + sin(angle) * radius;
        int yp = center + cos(angle) * radius;
        text.setString(name + " " + to_string(wins));
        text.setPosition(xp, yp - 60);
        if(isActive) text.setColor(Color(255,255,255));
        window->draw(text);
        text.setColor(Color(120,120,120));
		// Draw their cards.
        for(int i = 0; i < hand.size(); i++){
            drawer->getCard(hand.at(i))->setPosition(xp + ((double)i / (double)hand.size()) * 180 - 36, yp - 8);
            drawer->getCard(hand.at(i))->setScale(2,2);
            window->draw(*drawer->getCard(hand.at(i)));
            drawer->getCard(hand.at(i))->setScale(3,3);
        }
    }
	// returns if cardIn can be matched to cardtofind... wierd naming convention....
    bool matchCard(card& cardIn, card& cardtofind, COLOR topColor){
		// not sure why I split up this if statement. Could probably all be in one.
        if(cardIn.getColor() == topColor || cardIn.getColor() == WILD){
            return true;
        }
        if(cardIn.getNumber() == cardtofind.getNumber()){
            return true;
        }
        return false;
    }
	// will look through hand to see if a card can be matched.
    bool canPlayCard(card& c, COLOR topColor){
        int numwilds = 0;
        for(int i = 0; i < hand.size(); i++){ // look at each card.
            if(matchCard(hand.at(i), c, topColor)){ // if it matches,
                if(hand.at(i).getColor() == WILD){ // if it's a wild, we might want to save it, so don't return true just yet!!
                    numwilds += 1;
                    continue;
                } // if it's not a wild, then return true. Set saveWildCount so that it will reset and they will wait the next time they are stuck drawing.
				saveWildCount = 0;
                return true;
            }
        }
		if(numwilds > 1) return true; // if you have more than one wild, you might as well use the extra. Saving 2 wilds is always a bad strategy.
		if(numwilds == hand.size()){ // if that wild is your last card, then you won! Use it!
			saveWildCount = 0;
			return true;
		}
		if(numwilds == 1 && saveWildCount >= saveWildTurns && saveWildTurns > 0){ // player has already waited long enough and will now use the wild card.
			saveWildCount = 0;
			return true;
		}
		else{ // otherwise, hold off on using it, just draw a card instead of using it.
			saveWildCount++;
		}
        return false;
    }
    
    // plays a card that matches by color.
    card playByColor(card& c, COLOR topColor){
        int cind = -1;
        bool takeTop = evilness == 1;
        //Have card of same color?
        for(int i = 0; i < hand.size(); i++){
            if(hand.at(i).getColor() != topColor) continue;
            if(cind == -1) cind = i;
            else if(takeTop){
                if(hand.at(i).getNumber() > hand.at(cind).getNumber()) cind = i;
            }
            else{
                if(hand.at(i).getNumber() < hand.at(cind).getNumber()) cind = i;
            }
        }
        return playCard(cind);
    }
	// play a card that matches by number
    card playByNumber(card& c, COLOR topColor){
        int cind = -1;
        bool takeTop = evilness == 1;
        //Have card of same color?
        for(int i = 0; i < hand.size(); i++){
            if(hand.at(i).getNumber() != c.getNumber()) continue;
            if(cind == -1) cind = i;
            else if(takeTop){
                if(hand.at(i).getNumber() > hand.at(cind).getNumber()) cind = i;
            }
            else{
                if(hand.at(i).getNumber() < hand.at(cind).getNumber()) cind = i;
            }
        }
        return playCard(cind);
    }
	// play a wild card
    card playWild(card& c, COLOR topColor){
        int cind = -1;
        bool takeTop = evilness == 1;
        //Have card of same color?
        for(int i = 0; i < hand.size(); i++){
            if(hand.at(i).getColor() != WILD) continue;
            if(cind == -1) cind = i;
            else if(takeTop){
                if(hand.at(i).getNumber() > hand.at(cind).getNumber()) cind = i;
            }
            else{
                if(hand.at(i).getNumber() < hand.at(cind).getNumber()) cind = i;
            }
        }
        return playCard(cind);
    }
    
    // determines which card to use, it will ensure that they stick to their priority order
    card playBestCard(card& c, COLOR topColor){
		// this is kind of inefficient with all the ifs. Is there a better way?
        if(firstPriority == 'w'){
            if(countColor(WILD) > 0) return playWild(c,topColor);
        }
        else if(firstPriority == 'n'){
            if(countNumber(c.getNumber()) > 0) return playByNumber(c,topColor);
        }
        else{
            if(countColor(topColor) > 0) return playByColor(c,topColor);
        }
        
        if(secondPriority == 'w'){
            if(countColor(WILD) > 0) return playWild(c,topColor);
        }
        else if(secondPriority == 'n'){
            if(countNumber(c.getNumber()) > 0) return playByNumber(c,topColor);
        }
        else{
            if(countColor(topColor) > 0) return playByColor(c,topColor);
        }
        
        if((firstPriority == 'w' && secondPriority == 'n') || (firstPriority == 'n' && secondPriority == 'w')){
            if(countColor(topColor) > 0) return playByColor(c,topColor);
        }
        else if((firstPriority == 'w' && secondPriority == 'c') || (firstPriority == 'c' && secondPriority == 'w')){
            if(countNumber(c.getNumber()) > 0) return playByNumber(c,topColor);
        }
        else{
            if(countColor(WILD) > 0) return playWild(c,topColor);
        }
        
        
        if(countColor(topColor) > 0) return playByColor(c,topColor);
        if(countColor(topColor) > 0) return playByNumber(c,topColor);
        if(countColor(topColor) > 0) return playByColor(c,topColor);
    }
    
    // plays a card, ensures that card is removed from their hand.
    card playCard(int index){
        card returncard = hand.at(index);
        for(int i = 0; i < hand.size() - 1; i++){
            if(i < index) continue;
            hand.at(i) = hand.at(i+1);
        }
        hand.pop_back();
        return returncard;
    }
	// used when they use a wild to choose the best color to change it to.
    COLOR favoriteColor(){
        COLOR rColor = RED;
        int max = 0;
        if(countColor(RED) > max){
            max = countColor(RED);
            rColor = RED;
        }
        if(countColor(YELLOW) > max){
            max = countColor(YELLOW);
            rColor = YELLOW;
        }
        if(countColor(GREEN) > max){
            max = countColor(GREEN);
            rColor = GREEN;
        }
        if(countColor(BLUE) > max){
            max = countColor(BLUE);
            rColor = BLUE;
        }
        return rColor;
    }
	// counts cards of that color in hand
    int countColor(COLOR color){
        int count = 0;
        for(int i = 0; i < hand.size(); i++){
            if(hand.at(i).getColor() == color) count += 1;
        }
        return count;
    }
	// count cards of a specific number.
    int countNumber(int num){
        int count = 0;
        for(int i = 0; i < hand.size(); i++){
            if(hand.at(i).getNumber() == num) count += 1;
        }
        return count;
    }
};







/*
 deck class is more than just a collection of cards, all of the logic of playing takes place here!
*/
class deck{
public:
    bool pause = true;
    vector<card> drawdeck; // draw deck
    vector<card> discards; // discards. top card of discards is the one to match by!
	// NOTE: Maybe these could be stacks?
    vector<player> players; // vector of players.
    COLOR topColor; // keeps track of the current color.
	// NOTE: This is separate from the color of the top discard color because a wild is no color but there is still a color to match.
    bool clockwise = true; // direction of play. can be reversed!
    int numPlayers = 0; // keeps track of how many players there are
    int activePlayer = 0; // keeps track of which player's turn it is
    Texture cardBacktexture; // just a texture for the back of a card. purely for aesthetics
    Sprite cardBack;
    CardDrawer drawer; // card drawer to use
    int wintimer = 0; // a timer that controls how long to display the "[name] wins!" text
    RectangleShape rectangle; // rectangle that gets drawn under discard deck to show current color.
	
	// Constants for evilness. Maybe make this a global enum?
    static const int NICE = -1;
	static const int RANDOM = 0;
	static const int EVIL = 1;
	
	// constuctor
    deck(){
		// Seed the random number generator
        srand(time(NULL));
		// Add a 0 card for each color.
        drawdeck.push_back(card(RED,0));
        drawdeck.push_back(card(YELLOW,0));
        drawdeck.push_back(card(GREEN,0));
        drawdeck.push_back(card(BLUE,0));
		// Add the other cards 1-9, skip, reverse, draw
		// Note that 0 is loaded above because there is only 1 zero for each color
		// while there are two coppies of each other colored card
        for(int i = 1; i <= 12; i++){
            drawdeck.push_back(card(RED,i));
            drawdeck.push_back(card(YELLOW,i));
            drawdeck.push_back(card(GREEN,i));
            drawdeck.push_back(card(BLUE,i));
            drawdeck.push_back(card(RED,i));
            drawdeck.push_back(card(YELLOW,i));
            drawdeck.push_back(card(GREEN,i));
            drawdeck.push_back(card(BLUE,i));
        }
		// Load wilds. -2 is plain wild, -3 is draw 4.
        for(int i = 0; i < 4; i++){
            drawdeck.push_back(card(WILD,-3));
            drawdeck.push_back(card(WILD,-2));
        }
		// Shuffle the deck. This uses a custom shuffling algorithm.
        shuffle(6);
		
        topColor = RED;
        
        cardBacktexture.loadFromFile(resourcePath() + "cards/back.png");
        cardBack.setTexture(cardBacktexture);
        cardBack.setScale(3,3);
        
    }
	
    void loadPlayers(){
		// load players into the players vector. player constructor is:
		// player("name", evilness, saveWildTurns, firstPriority, secondPriority)
		
		// ************************************************************************************************** PUT PLAYERS IN HERE!!!!
		players.push_back(player("Evan",NICE,1,'n','c'));
		players.push_back(player("Katrina",RANDOM,0,'c','n'));
		players.push_back(player("Jacob",EVIL,2,'c','n'));
		// ************************************************************************************************** PUT PLAYERS IN HERE!!!!
		
        numPlayers = players.size();
		
		// give players 7 cards each.
        for(int i = 0; i < numPlayers * 7; i++){
            players.at(i % numPlayers).pickUp(draw());
        }
		// flip over the top card
        placeCard(draw());
		// if it's a wild you have to re-shuffle.
        while(discards.back().getColor() == WILD){
            drawdeck.push_back(discards.back());
            shuffle(2);
            placeCard(draw());
        }
        topColor = discards.back().getColor();
    }
	
	// shuffle the discards into the draw deck. Leave the top card though
    void reshuffle(){
        card topCard = discards.at(discards.size() - 1);
        for(int i = 0; i < discards.size() - 1; i++){
            drawdeck.push_back(discards.at(i));
        }
        shuffle(4);
        discards.clear();
        discards.push_back(topCard);
    }
	bool stuck = false;
	// returns the top draw card. Reshuffles if there aren't any cards to draw.
    card draw(){
		if(drawdeck.size()==0 && discards.size() < 5) cout << drawdeck.size() << " " << discards.size() << " " << activePlayer << endl;
		
		if(drawdeck.size() > 1) stuck = false;
        if(drawdeck.size() == 0){
            if(discards.size() == 1){ // did you know that you could tie in uno? fun fact.
                reset();
                cout << "\n########### NOBODY WINS! #############\n";
                wintimer = 2;
                return discards.back();
            }
			else if(discards.size() == 2){ // oops. got stuck. all the cards are in the player's hands and they will just keep using the same card over and over.
				if(activePlayer == 0){
					if(!stuck) stuck = true;
					else{
						reset();
						cout << "\n########### NOBODY WON: STUCK #############\n";
						wintimer = 2;
						return discards.back();
					}
				}
			}
            reshuffle();
        }
        card returncard = drawdeck.at(drawdeck.size() - 1);
        drawdeck.pop_back();
        return returncard;
    }
    // adds a card to the discard pile
    void placeCard(card c){
        discards.push_back(c);
    }
    // same as the player matchCard funciton. Not sure why this is here actually...
    bool matchCard(card c){
        if(c.getColor() == topColor || c.getColor() == WILD){
            return true;
        }
        if(c.getNumber() == discards.back().getNumber()){
            return true;
        }
        return false;
    }
    // draws everything to the screen.
    void draw(RenderWindow* window, Text& text, double radius, int width, int height){
		// if someone won, just display their name for a bit
        if(wintimer == 1){
            reset();
            wintimer = 0;
            return;
        }
        if(wintimer > 1){
            for(int i = 0; i < players.size(); i++){
                if(players.at(i).hand.size() == 0){
                    text.setString(players.at(i).name + " Wins!");
                    text.setPosition(50, 50);
                    window->draw(text);
                    //cout << wintimer << " ";
                }
            }
            wintimer -= 1;
            return;
        }
        
        
        rectangle.setPosition(width / 2 - 20, height / 2 - (16 * 3) / 2 - 3);
        rectangle.setSize(Vector2f(48,48+6));
        rectangle.setFillColor(Color(255,0,0));
        if(topColor == YELLOW) rectangle.setFillColor(Color(255,255,0));
        if(topColor == GREEN) rectangle.setFillColor(Color(0,255,0));
        if(topColor == BLUE) rectangle.setFillColor(Color(0,0,255));
        window->draw(rectangle);
        
        cardBack.setPosition(width / 2 - 80, height / 2 - (16 * 3) / 2);
        window->draw(cardBack);
        drawer.getCard(discards.back())->setPosition(width / 2 - 20, height / 2 - (16 * 3) / 2);
        window->draw(*drawer.getCard(discards.back()));
        
        
        for(int i = 0; i < players.size(); i++){
            double angle =((double)i / (double)players.size()) * (2.0 * 3.1415) + 0.0;
            players.at(i).draw(window, text, height / 2, angle, radius, &drawer, i == activePlayer);
        }
    }
    
    // custom shuffling algorithm to mimic real shuffling
    void shuffle(int times){
        for(int i = 0; i < times; i++){
			// Number of cards we have to keep track of
			int size = drawdeck.size();
			
			// Split the drawdeck in half into two temporary decks
            vector<card> temp[2];
            for(int i = 0; i < size; i++){
                if(i < drawdeck.size() / 2){
                    temp[0].push_back(drawdeck.at(i));
                }
				else{
					temp[1].push_back(drawdeck.at(i));
				}
            }
            drawdeck.clear();
			
			// While there are still cards left, randomly choose one of the temporary decks
			// and move a card from that deck back to the drawdeck
			int count[2] = {0,0};
            for(int i = 0; i < size; i++){
				bool takefrom = rand() % 2;
				if(count[takefrom] == temp[takefrom].size()) takefrom = !takefrom;
				drawdeck.push_back(temp[takefrom].at(count[takefrom]));
				count[takefrom]++;
            }
			
        }
    }
	
	// increments activePlayer. might have to do this backwards
    void nextPlayer(){
        if(!clockwise) activePlayer += 1;
        else activePlayer -= 1;
        if(activePlayer == players.size()) activePlayer = 0;
        if(activePlayer < 0) activePlayer = players.size() - 1;
    }
    
    // start a new game!
    void reset(){
		// clear hands back into discards
        for(int i = 0; i < players.size(); i++){
            for(int j = 0; j < players.at(i).hand.size(); j++){
                placeCard(players.at(i).hand.at(j));
            }
            players.at(i).hand.clear();
        }
		// put all discards into drawdeck
        for(int i = 0; i < discards.size(); i++){
            drawdeck.push_back(discards.at(i));
        }
		// clear discards
        discards.clear();
		// shuffle
        shuffle(6);
		// give cards to players
        for(int i = 0; i < players.size() * 7; i++){
            players.at(i % players.size()).pickUp(draw());
        }
		// flip over the top card
        placeCard(draw());
		// if it's a wild, reshuffle
        while(discards.back().getColor() == WILD){
            drawdeck.push_back(discards.back());
            discards.clear();
            shuffle(2);
            placeCard(draw());
        }
        topColor = discards.back().getColor();
		// silly way of saying go to the previous player
        if(clockwise) clockwise = false;
        else clockwise = true;
        nextPlayer();
		// play starts clockwise
        clockwise = true;
    }
    
    // calls on each update
    void update(){
        if(wintimer > 0){
            return;
        }
		// check if someone won
        for(int i = 0; i < players.size(); i++){
            if(players.at(i).hand.size() == 0){
                players.at(i).wins += 1;
                wintimer = 30;
                return;
            }
        }
		// play a card and do special effects of that card.
        if(players.at(activePlayer).canPlayCard(discards.back(), topColor)){
            placeCard(players.at(activePlayer).playBestCard(discards.back(), topColor));
            if(discards.back().getColor() != WILD){
                topColor = discards.back().getColor();
            }
            else{ // someone played a wild
                topColor = players.at(activePlayer).favoriteColor();
                if(discards.back().getNumber() == -2){ // it's a draw 4 haha
                    nextPlayer();
                    players.at(activePlayer).pickUp(draw());
                    players.at(activePlayer).pickUp(draw());
                    players.at(activePlayer).pickUp(draw());
                    players.at(activePlayer).pickUp(draw());
                }
            }
            if(discards.back().getNumber() == 10){ // skip
                nextPlayer();
            }
            else if(discards.back().getNumber() == 11){ // reverse
                if(clockwise) clockwise = false;
                else clockwise = true;
            }
            else if(discards.back().getNumber() == 12){ // draw 2
                nextPlayer();
                players.at(activePlayer).pickUp(draw());
                players.at(activePlayer).pickUp(draw());
            }
            nextPlayer();
            return;
        }
        else{
            players.at(activePlayer).pickUp(draw()); // draw a card if they can't play
        }
    } 
    
    
    
    
    
    // does the same as update, except it will continuously update until the game finishes without user input
	// NOTE: this is not very DRY... maybe find a way to combine the functions?
    void playGame(){
        bool gameWon = false;
        while(!gameWon){
            if(wintimer == 2){
                cout << "nobody won\n";
                reset();
                wintimer = 0;
                break;
            }
            if(players.at(activePlayer).canPlayCard(discards.back(), topColor)){
                placeCard(players.at(activePlayer).playBestCard(discards.back(), topColor));
                if(discards.back().getColor() != WILD){
                    topColor = discards.back().getColor();
                }
                else{
                    topColor = players.at(activePlayer).favoriteColor();
                    if(discards.back().getNumber() == -2){
                        nextPlayer();
                        players.at(activePlayer).pickUp(draw());
                        players.at(activePlayer).pickUp(draw());
                        players.at(activePlayer).pickUp(draw());
                        players.at(activePlayer).pickUp(draw());
                    }
                }
                if(discards.back().getNumber() == 10){
                    nextPlayer();
                }
                else if(discards.back().getNumber() == 11){
                    if(clockwise) clockwise = false;
                    else clockwise = true;
                }
                else if(discards.back().getNumber() == 12){
                    nextPlayer();
                    players.at(activePlayer).pickUp(draw());
                    players.at(activePlayer).pickUp(draw());
                }
                nextPlayer();
            }
            else{
                players.at(activePlayer).pickUp(draw());
            }
            for(int i = 0; i < players.size(); i++){
                if(players.at(i).hand.size() == 0){
                    players.at(i).wins += 1;
                    reset();
                    gameWon = true;
                }
            }
        }
        
        
    }
    
};




















#endif /* deck_hpp */
