#include <RtMidi.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "synthesis.hpp"
#include "drawFunctions.hpp"
#include "readFunctions.hpp"

#include "SelbaWard/ProgressBar.hpp"

RtMidiOut midiOut;

using namespace sf;
using namespace sw;

ProgressBar fileProgress(Vector2f(600, 40));

vector<string> files;
uint64_t noteIndex, metaIndex, systemIndex;
uint64_t globalLength;
uint16_t PPQN;
uint64_t MIDITime;
uint64_t lastMIDITime;
bool activeNotes[16][128];

int selectedFile = -1;
int selectedDevice = -1;

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
int speedX[9] = {124, 168, 212, 256, 300, 344, 388, 432, 800};
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

int displaySelectionScreen(RenderWindow &select)
{
  select.setFramerateLimit(60);

  listMidiDevices();

  fileProgress.setShowBackgroundAndFrame(true);

  fileProgress.setPosition(Vector2f(500, 30));

  fileProgress.setFrameThickness(1);;

  fileProgress.setBackgroundColor(Color::Black);

  fileProgress.setColor(Color::White);

  fileProgress.setFrameColor(Color::White);

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

    if (m.x > 1180 && m.x < 1580 && m.y > 10 && m.y < 40 && (files.size() > 0 && MIDIDevices.size() > 0 && selectedDevice != -1 && selectedFile != -1))
    {
      startHover = true;
      if (Mouse::isButtonPressed(sf::Mouse::Left))
      {
        select.close();
        return 1;
      }
    }

    if (m.x > 1180 && m.x < 1580 && m.y > 50 && m.y < 80)
    {
      exitHover = true;
      if (Mouse::isButtonPressed(sf::Mouse::Left))
      {
        return 0;
      }
    }

    select.draw(drawRect(5, 90, 1590, 1, Color::White));
    select.draw(drawText(font, 5, 5, 80, "TriMIDI", Color::White, 1));
    select.draw(drawText(font, 330, 5, 20, "v.1.3.3", Color::White, 1));

    select.draw(drawRect(1190, 10, 400, 30, Color(0, 100 + (startHover * 50) - ((files.size() == 0 || MIDIDevices.size() == 0 || selectedDevice == -1 || selectedFile == -1) * 50), 0)));
    select.draw(drawRect(1190, 50, 400, 30, Color(100 + (exitHover * 50), 0, 0)));

    select.draw(drawText(font, 1390, 16, 16, "Start", Color::White, 0));
    select.draw(drawText(font, 1390, 56, 16, "Exit", Color::White, 0));

    if (files.size() > 0)
    {
      for (int i = 0; i < files.size(); i++)
      {
        bool hover = false;
        if (m.x > 40 && m.x < 800 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(sf::Mouse::Left))
          {
            selectedFile = i;
          }
        }
        select.draw(drawRect(40, i * 21 + 100, 760, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedFile) * 70))));
        select.draw(drawText(font, 40, i * 21 + 100, 18, files[i], Color::White, 1));
      }
    }
    else
    {
      select.draw(drawText(font, 400, 120, 20, "No MIDI Files Found", Color::White, 0));
    }

    if (MIDIDevices.size() > 0)
    {
      for (int i = 0; i < MIDIDevices.size(); i++)
      {
        bool hover = false;
        if (m.x > 840 && m.x < 1560 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(sf::Mouse::Left))
          {
            selectedDevice = i;
          }
        }
        select.draw(drawRect(840, i * 21 + 100, 720, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedDevice) * 70))));
        select.draw(drawText(font, 865, i * 21 + 100, 18, MIDIDevices[i], Color::White, 1));
        select.draw(drawSprite(midi, 841, i * 21 + 101, 2));
      }
    }
    else
    {
      select.draw(drawText(font, 1150, 120, 20, "No MIDI Devices Found", Color::White, 0));
    }

    select.draw(fileProgress);

    select.display();
  }
  return 0;
}

int displayPlayerScreen()
{
  RenderWindow window(sf::VideoMode(1600, 1000), "Player", Style::Titlebar);
  window.setFramerateLimit(60);

  globalLength = notes[notes.size() - 1][0];
  float step = 1540.0 / (float)globalLength;

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

    window.draw(drawRect(10, 10, 1560, 250, Color(15, 15, 15)));
    window.draw(drawSprite(grid, 20, 20, 2));

    window.draw(drawText(font, 20, 210, 32, to_string((int)Tempo) + " BPM (" + to_string(Tempo * speeds[speedIndex]) + ")", Color::White, 1));
    window.draw(drawRect(10, 270, 1560, 20, Color(60, 60, 60)));
    int x = ((float)step * (float)MIDITime) + 10;
    window.draw(drawSprite(marker, x - 6, 270, 2));
    if (m.x > x - 10 && m.x < x + 10 && m.y > 260 && m.y < 280 || seekT)
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
          window.draw(drawRect(22 + (j * 12), 22 + (i * 12), 10, 10, midiColors[i]));
        }
      }
    }

    window.draw(drawRect(10, 300, 1580, 1, Color::White));

    if (playing)
      window.draw(drawSprite(play, 20, 310, 4));
    else
      window.draw(drawSprite(stop, 20, 310, 4));

    play.setColor(sf::Color(255, 255, 255));
    stop.setColor(sf::Color(255, 255, 255));

    if (m.x > 20 && m.x < 64 && m.y > 310 && m.y < 354)
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

    window.draw(drawSprite(speed, 80, 310, 4));
    window.draw(drawText(font, 80, 360, 32, speedNames[speedIndex], Color::White, 1));
    x = (speedIndex * 40) + 126;
    window.draw(drawCircle(x, 332, 10, Color::Red));
    if(m.x > 80 && m.x < 452 && m.y > 310 && m.y < 354){
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
    if(m.x > 10 && m.x < 410 && m.y > 930 && m.y < 990){
      backHover = true;
      if(Mouse::isButtonPressed(Mouse::Left)){
        return 1;
      }
    }
    window.draw(drawRect(10, 930, 400, 60, Color(100 + (backHover * 50), 0, 0)));
    window.draw(drawText(font, 210, 940, 32, "Back", Color::White, 0));

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
    if(noteIndex == notes.size() - 1 && systemIndex == systemMessages.size() - 1 && metaIndex == meta.size() - 1)
      return 1;
  }
  return 0;
}