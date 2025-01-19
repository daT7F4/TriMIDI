#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace sf;

Font font;
Texture texture;
Sprite grid, grid2, play, stop, marker, midi, speed;

void initFont()
{
  font.loadFromFile("./assets/Doto-VariableFont_ROND,wght.ttf");
}

Text drawText(int x, int y, int size, string t, Color color, bool centerAllign)
{
  Text text;
  text.setFont(font);
  text.setString(t);
  text.setCharacterSize(size);
  text.setFillColor(color);
  FloatRect bounds = text.getLocalBounds();
  if (centerAllign)
    text.setPosition(Vector2f(x, y));
  else
    text.setPosition(Vector2f(x - ((int)bounds.width / 2), y));
  return text;
}

RectangleShape background(int x, int y, int w, int h)
{
  RectangleShape ground;
  ground.setPosition(Vector2f(x, y));
  ground.setSize(Vector2f(w, h));
  ground.setFillColor(Color(50, 50, 50, 150));
  return ground;
}

CircleShape drawCircle(int x, int y, int r, Color color)
{
  CircleShape circle(r);
  circle.setPosition(Vector2f(x - r, y - r));
  circle.setFillColor(color);
  return circle;
}

RectangleShape drawRect(int x, int y, int w, int h, Color color)
{
  RectangleShape rect;
  rect.setPosition(Vector2f(x, y));
  rect.setSize(Vector2f(w, h));
  rect.setFillColor(color);
  return rect;
}

void loadSprites(){
  if (!texture.loadFromFile("./assets/texture.png"))
  {
    cerr << "Failed to load texture" << endl;
  }
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

Sprite drawSprite(Sprite &sprite, int x, int y, float scale){
  sprite.setPosition(Vector2f(x, y));
  sprite.setScale(Vector2f(scale, scale));
  return sprite;
}

