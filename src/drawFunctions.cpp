#include "drawFunctions.hpp"
#include <SFML/Graphics.hpp>

sf::Font font, thiccfont;
sf::Texture texture;

void Button::InitButton(){
  button.setPosition(sf::Vector2f(x, y));
  button.setSize(sf::Vector2f(w, h));

  Label.setFont(font);
  Label.setString(LabelText);
  Label.setCharacterSize(16);
  Label.setFillColor(sf::Color::White);
  sf::FloatRect bounds = Label.getLocalBounds();
  if(centerAllign)
    Label.setPosition(sf::Vector2f(x - ((int)bounds.width / 2) + (w / 2), y - bounds.height + (h / 2)));
  else
    Label.setPosition(sf::Vector2f(x + textOffset, y));
}

void Button::DrawButton(sf::RenderWindow &window)
{
  switch(mode){
    case 0:
      button.setFillColor(Locked);
      break;
    case 1:
      button.setFillColor(Open);
      break;
    case 2:
      button.setFillColor(Hover);
      break;
  }
  window.draw(button);
  window.draw(Label);
}

void Label::InitText(sf::Font &font){
  Label.setFont(font);
  Label.setString(text);
  Label.setCharacterSize(size);
  Label.setFillColor(color);
  sf::FloatRect bounds = Label.getLocalBounds();
  if(allign == "l"){
     Label.setPosition(sf::Vector2f(x, y));
  } else if(allign == "c"){
    Label.setPosition(sf::Vector2f(x - ((int)bounds.width / 2), y));
  } else if(allign == "r"){
    Label.setPosition(sf::Vector2f(x - bounds.width, y));
  }
}

void Label::DrawText(sf::RenderWindow &window){
  window.draw(Label);
}

void Circle::InitCircle(){
  circle.setRadius(r);
  circle.setPosition(sf::Vector2f(x - r, y - r));
  circle.setFillColor(color);
}

void Circle::DrawCircle(sf::RenderWindow &window){
  window.draw(circle);
}

void Rectangle::InitRect(){
  rectangle.setPosition(sf::Vector2f(x, y));
  rectangle.setSize(sf::Vector2f(w, h));
  rectangle.setFillColor(color);
}

void Rectangle::DrawRect(sf::RenderWindow &window){
  window.draw(rectangle);
}

void Sprite::InitSprite(sf::IntRect spriteRect){
  sprite.setTexture(texture);
  sprite.setTextureRect(spriteRect);
  sprite.setScale(sf::Vector2f(scale, scale));
  sprite.setPosition(sf::Vector2f(x, y));
  sprite.setColor(color);
}

void Sprite::DrawSprite(sf::RenderWindow &window){
  window.draw(sprite);
}