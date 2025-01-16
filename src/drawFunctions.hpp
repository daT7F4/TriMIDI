#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

int keys[12] = {0,3,1,3,2,0,3,1,3,1,3,2}; // 0 right white, 1 center white, 2, left white, 3 black 

Font font;

void initFont()
{
  font.loadFromFile("./../Doto-VariableFont_ROND,wght.ttf");
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
    text.setPosition(Vector2f(x - (bounds.width / 2), y));
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