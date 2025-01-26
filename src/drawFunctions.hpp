#include <string>
#include <vector>
#include <iostream>
#include <string>

#ifndef DRAWFUNCTIONS_HPP
#define DRAWFUNCTIONS_HPP

using namespace std;

sf::Font font, thiccfont;
sf::Texture texture;

class Button{
  public:
    int x, textOffset, y, w, h, mode; // 0 - Locked, 1 - Open, 2 - Hover
    bool centerAllign;
    string LabelText;
    sf::Color Locked, Open, Hover;
    void InitButton();
    void DrawButton(sf::RenderWindow &window);
  private:
    sf::RectangleShape button;
    sf::Text Label;
};

class Label{
  public: 
    int x, y, size;
    string text, allign;
    sf::Color color;
    void InitText(sf::Font &font);
    void DrawText(sf::RenderWindow &window);
  private:
    sf::Text Label;
};

class Circle{
  public:
    int x, y, r;
    sf::Color color;
    void InitCircle();
    void DrawCircle(sf::RenderWindow &window);
  private:
    sf::CircleShape circle;
};

class Rectangle{
  public:
    int x, y, w, h;
    sf::Color color;
    void InitRect();
    void DrawRect(sf::RenderWindow &window);
  private:
    sf::RectangleShape rectangle;
};

class Sprite{
  public:
    int x, y, scale;
    void InitSprite(sf::IntRect spriteRect);
    void DrawSprite(sf::RenderWindow &window);
  private:
    sf::Sprite sprite;
};

/*

void loadSprites(){
  
  grid.setTexture(texture);
  grid.setTextureRect(IntRect(0, 0, 769, 97));
  play.setTexture(texture);
  play.setTextureRect(IntRect(0, 98, 11, 11));
  stop.setTexture(texture);
  stop.setTextureRect(IntRect(12, 98, 11, 11));
  marker.setTexture(texture);
  marker.setTextureRect(IntRect(24, 98, 7, 11));
  midi.setTexture(texture);
  midi.setTextureRect(IntRect(32, 98, 9, 9));
  speed.setTexture(texture);
  speed.setTextureRect(IntRect(42, 98, 93, 11));
}

*/

#endif;

