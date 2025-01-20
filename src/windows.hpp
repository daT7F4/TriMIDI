#include <RtMidi.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "synthesis.hpp"
#include "drawFunctions.hpp"
#include "readFunctions.hpp"

RtMidiOut midiOut;

using namespace sf;

vector<string> files;
uint64_t noteIndex, metaIndex, systemIndex;
uint64_t globalLength;
uint16_t PPQN;
uint64_t MIDITime;
uint64_t lastMIDITime;
bool activeNotes[16][128];

int selectedFile;
int selectedDevice;

bool seekT = false;
bool playT = false;
bool playing = true;

float Tempo;

Color midiColors[16] = {
    Color(255, 87, 34),  // Red-Orange
    Color(76, 175, 80),  // Green
    Color(33, 150, 243), // Blue
    Color(255, 235, 59), // Yellow
    Color(156, 39, 176), // Purple
    Color(244, 67, 54),  // Red
    Color(0, 188, 212),  // Cyan
    Color(205, 220, 57), // Lime
    Color(121, 85, 72),  // Brown
    Color(255, 193, 7),  // Amber
    Color(63, 81, 181),  // Indigo
    Color(139, 195, 74), // Light Green
    Color(255, 152, 0),  // Orange
    Color(103, 58, 183), // Deep Purple
    Color(96, 125, 139), // Blue Grey
    Color(233, 30, 99)   // Pink
};

float speeds[8] = {0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2};
string speedNames[8] = {"0.25x", "0.5x", "0.75x", "1x", "1.25x", "1.5x", "1.75x", "2x"};
int speedX[9] = {62, 84, 106, 128, 150, 172, 194, 216, 800};
int speedIndex = 3;

void updateNotes(bool reverse)
{
  thirdytwo2eight(notes[noteIndex][1]);
  if (byte4)
  {
    playNote(midiOut, byte1, byte2, byte3);
    activeNotes[byte1][byte2] = !reverse;
  }
  else
  {
    stopNote(midiOut, byte1, byte2);
    activeNotes[byte1][byte2] = reverse;
  }
}

void updateSystem()
{
  thirdytwo2eight(systemMessages[systemIndex][1]);
  switch (byte1)
  {
  case 0:
    sendProgramChange(midiOut, byte2, byte3);
    break;
  }
}

int displaySelectionScreen()
{
  selectedFile = -1;
  selectedDevice = -1;

  ContextSettings settings;
  settings.antialiasingLevel = 0;
  RenderWindow select(sf::VideoMode(800, 600), "Selection Screen", Style::Default, settings);
  select.setFramerateLimit(60);

  listMidiDevices();

  while (select.isOpen())
  {
    Event event;
    while (select.pollEvent(event))
    {
      if (event.type == Event::Closed)
      {
        select.close();
        return 0;
      }
    }
    select.clear();
    Vector2i m = Mouse::getPosition(select);

    bool startHover = false, exitHover = false;

    if (m.x > 590 && m.x < 790 && m.y > 10 && m.y < 40 && (files.size() > 0 && MIDIDevices.size() > 0 && selectedDevice != -1 && selectedFile != -1))
    {
      startHover = true;
      if (Mouse::isButtonPressed(sf::Mouse::Left))
      {
        select.close();
        return 1;
      }
    }

    if (m.x > 590 && m.x < 790 && m.y > 50 && m.y < 80)
    {
      exitHover = true;
      if (Mouse::isButtonPressed(sf::Mouse::Left))
      {
        return 0;
      }
    }

    select.draw(drawRect(5, 90, 790, 1, Color::White));
    select.draw(drawText(5, 5, 80, "TriMIDI", Color::White, 1));
    select.draw(drawText(330, 5, 20, "v.1.3.2.1", Color::White, 1));

    select.draw(drawRect(590, 10, 200, 30, Color(0, 100 + (startHover * 50) - ((files.size() == 0 || MIDIDevices.size() == 0 || selectedDevice == -1 || selectedFile == -1) * 50), 0)));
    select.draw(drawRect(590, 50, 200, 30, Color(100 + (exitHover * 50), 0, 0)));

    select.draw(drawText(690, 16, 16, "Start", Color::White, 0));
    select.draw(drawText(690, 56, 16, "Exit", Color::White, 0));

    if (files.size() > 0)
    {
      for (int i = 0; i < files.size(); i++)
      {
        bool hover = false;
        if (m.x > 40 && m.x < 390 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(sf::Mouse::Left))
          {
            selectedFile = i;
          }
        }
        select.draw(drawRect(40, i * 21 + 100, 350, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedFile) * 70))));
        select.draw(drawText(40, i * 21 + 100, 18, files[i], Color::White, 1));
      }
    }
    else
    {
      select.draw(drawText(215, 120, 20, "No MIDI Files Found", Color::White, 0));
    }

    if (MIDIDevices.size() > 0)
    {
      for (int i = 0; i < MIDIDevices.size(); i++)
      {
        bool hover = false;
        if (m.x > 400 && m.x < 750 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(sf::Mouse::Left))
          {
            selectedDevice = i;
          }
        }
        select.draw(drawRect(400, i * 21 + 100, 350, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedDevice) * 70))));
        select.draw(drawText(425, i * 21 + 100, 18, MIDIDevices[i], Color::White, 1));
        select.draw(drawSprite(midi, 401, i * 21 + 101, 2));
      }
    }
    else
    {
      select.draw(drawText(575, 120, 20, "No MIDI Devices Found", Color::White, 0));
    }

    select.display();
  }
  return 0;
}

int displayPlayerScreen()
{
  RenderWindow window(sf::VideoMode(800, 600), "Player");
  window.setFramerateLimit(60);

  globalLength = notes[notes.size() - 1][0];
  float step = 770.0 / (float)globalLength;

  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed){
        window.close();
        return 0;
      }
    }

    Vector2i m = Mouse::getPosition(window);

    window.clear();

    window.draw(drawRect(5, 5, 780, 125, Color(15, 15, 15)));
    window.draw(drawSprite(grid, 10, 10, 1));

    window.draw(drawText(10, 105, 16, to_string((int)Tempo) + " BPM (" + to_string(Tempo * speeds[speedIndex]) + ")", Color::White, 1));
    window.draw(drawRect(5, 135, 780, 10, Color(60, 60, 60)));
    int x = ((float)step * (float)MIDITime) + 5;
    window.draw(drawSprite(marker, x - 3, 135, 1));
    if (m.x > x - 5 && m.x < x + 5 && m.y > 130 && m.y < 140 || seekT)
    {
      seekT = false;
      if (Mouse::isButtonPressed(sf::Mouse::Left))
      {
        seekT = true;
        MIDITime = (((float)m.x - 10.0) / (float)step);
        if (MIDITime != lastMIDITime)
        {
          while (notes[noteIndex][0] > MIDITime)
          {
            updateNotes(true);
            noteIndex--;
          }
          while (systemMessages[systemIndex][0] > MIDITime)
          {
            updateSystem();
            systemIndex--;
          }
          lastMIDITime = MIDITime;
        }
      }
    }

    MIDITime = min(MIDITime, (uint64_t)globalLength);
    for (int i = 0; i < 16; i++)
    {
      for (int j = 0; j < 128; j++)
      {
        if (activeNotes[i][j])
        {
          window.draw(drawRect(11 + (j * 6), 11 + (i * 6), 5, 5, midiColors[i]));
        }
      }
    }

    window.draw(drawRect(5, 150, 790, 1, Color::White));

    if (playing)
      window.draw(drawSprite(play, 10, 155, 2));
    else
      window.draw(drawSprite(stop, 10, 155, 2));

    play.setColor(sf::Color(255, 255, 255));
    stop.setColor(sf::Color(255, 255, 255));

    if (m.x > 10 && m.x < 30 && m.y > 150 && m.y < 170)
    {
      if(playing)
        play.setColor(sf::Color(160, 160, 160));
      else
        stop.setColor(sf::Color(160, 160, 160));

      if (Mouse::isButtonPressed(sf::Mouse::Left) && !playT)
      {
        playing = !playing;
        playT = true;
      }
      if (!Mouse::isButtonPressed(sf::Mouse::Left))
        playT = false;
    }

    window.draw(drawSprite(speed, 40, 155, 2));
    window.draw(drawText(40, 180, 16, speedNames[speedIndex], Color::White, 1));
    x = (speedIndex * 20) + 63;
    window.draw(drawCircle(x, 166, 5, Color::Red));
    if(m.x > 40 && m.x < 226 && m.y > 155 && m.y < 177){
      if(Mouse::isButtonPressed(sf::Mouse::Left)){
        int lowestIndex = 8;
        for(int i = 0; i < 8; i++){
          if(abs(speedX[i] - m.x) < abs(speedX[lowestIndex] - m.x)){
            lowestIndex = i;
          }
        }
        speedIndex = lowestIndex;
      } 
    }

    bool backHover = false;
    if(m.x > 5 && m.x < 205 && m.y > 565 && m.y < 695){
      backHover = true;
      if(Mouse::isButtonPressed(Mouse::Left)){
        return 1;
      }
    }
    window.draw(drawRect(5, 565, 200, 30, Color(100 + (backHover * 50), 0, 0)));
    window.draw(drawText(105, 570, 16, "Back", Color::White, 0));

    window.display();

    if (playing)
      MIDITime += (((float)Tempo * (float)PPQN) / 60.0) * (1.0 / 60.0) * speeds[speedIndex];

    while (notes[noteIndex][0] < MIDITime && noteIndex < notes.size() - 1)
    {
      updateNotes(false);
      noteIndex++;
    }

    if (noteIndex == notes.size())
      window.close();

    while (meta[metaIndex][0] < MIDITime && metaIndex < meta.size() - 1)
    {
      metaIndex++;
      Tempo = 60000000.0 / meta[metaIndex][1];
    }
    while (systemMessages[systemIndex][0] < MIDITime && systemIndex < systemMessages.size() - 1)
    {
      updateSystem();
      systemIndex++;
    }
  }
  return 0;
}