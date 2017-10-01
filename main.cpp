#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>

#include "ResourcePath.hpp"
#include "deck.hpp"

using namespace std;
using namespace sf;


RenderWindow* window;
deck unoDeck;
Font font;
Text text;
int width = 48 * 13; // 624
int height = 24 + 48 * 9; // 456
int countdown = 0;

void startup(){
	// set up the screen and stuff
    window = new RenderWindow(VideoMode(width, height), "Uno Player");
    window->setFramerateLimit(60);
    
    window->setSize(Vector2u(width * 2, height * 2));
    window->setPosition(Vector2i(640,480));
    Image icon;
    if (icon.loadFromFile(resourcePath() + "icon.png")) {
        window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }
    font.loadFromFile(resourcePath() + "europe.ttf");
    text.setFont(font);
    text.setCharacterSize(96);
    text.setColor(Color::White);
    text.setScale(0.5,0.5);
}

void display(){
    window->clear(Color(80,80,80));
    unoDeck.draw(window,text,180,width,height);
    window->display();
}
void update(){
    countdown -= 1;
    if(countdown > 0) return;
    if(sf::Keyboard::isKeyPressed(Keyboard::Space)){
        unoDeck.update();
        countdown = 20;
    }
}

// start by playing a certain number of games and then going to user updated game
int main(){
    startup();
    unoDeck.loadPlayers();
    for(int i = 0; i < 1000; i++){
        unoDeck.playGame();
    }
    while(window->isOpen()){
        Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
                return 0;
            }
        }
        update();
        display();
    }

    return 0;
}
