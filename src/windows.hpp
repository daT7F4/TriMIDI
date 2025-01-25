#include <RtMidi.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "synthesis.hpp"
#include "drawFunctions.hpp"
#include "readFunctions.hpp"

RtMidiOut midiOut;

using namespace sf;

Clock measure;
Vector2i m;

vector<string> files;
uint64_t globalIndex;
uint64_t globalLength;
uint16_t PPQN;
uint64_t MIDITime;
uint64_t lastMIDITime;
uint64_t NotesPerSecond;

int selectedFile = -1;
int selectedDevice = -1;

bool seekT = false;
bool playT = false;
bool playing = true;

float Tempo;
double fps = 60.0;

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

bool inside(int x1, int x2, int y1, int y2)
{
  return m.x > x1 && m.x < x2 && m.y > y1 && m.y < y2;
}

int displaySelectionScreen()
{
  RenderWindow select(VideoMode(1600, 1000), "Selection Screen", Style::Titlebar);

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
    if (Keyboard::isKeyPressed(Keyboard::Escape))
    {
      select.close();
      return 0;
    }
    select.clear();

    m = Mouse::getPosition(select);

    bool startHover = false, exitHover = false;

    if (inside(1180, 1580, 10, 40) && (files.size() > 0 && MIDIDevices.size() > 0 && selectedDevice != -1 && selectedFile != -1))
    {
      startHover = true;
      if (Mouse::isButtonPressed(Mouse::Left))
      {
        select.close();
        return 1;
      }
    }

    if (inside(1180, 1580, 50, 80))
    {
      exitHover = true;
      if (Mouse::isButtonPressed(Mouse::Left))
      {
        return 0;
      }
    }

    select.draw(drawRect(5, 90, 1590, 1, Color::White));
    select.draw(drawText(thiccfont, 5, 5, 80, "TriMIDI", Color::White, "l"));
    select.draw(drawText(font, 330, 5, 20, "v.1.4", Color::White, "l"));

    select.draw(drawRect(1190, 10, 400, 30, Color(0, 100 + (startHover * 50) - ((files.size() == 0 || MIDIDevices.size() == 0 || selectedDevice == -1 || selectedFile == -1) * 50), 0)));
    select.draw(drawRect(1190, 50, 400, 30, Color(100 + (exitHover * 50), 0, 0)));

    select.draw(drawText(thiccfont, 1390, 16, 16, "Start", Color::White, "c"));
    select.draw(drawText(thiccfont, 1390, 56, 16, "Exit", Color::White, "c"));

    if (files.size() > 0)
    {
      for (int i = 0; i < files.size(); i++)
      {
        bool hover = false;
        if (m.x > 40 && m.x < 800 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(Mouse::Left))
          {
            selectedFile = i;
          }
        }
        select.draw(drawRect(40, i * 21 + 100, 760, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedFile) * 70))));
        select.draw(drawText(font, 40, i * 21 + 100, 18, files[i], Color::White, "l"));
      }
    }
    else
    {
      select.draw(drawText(thiccfont, 400, 120, 20, "No MIDI Files Found", Color::White, "l"));
    }

    if (MIDIDevices.size() > 0)
    {
      for (int i = 0; i < MIDIDevices.size(); i++)
      {
        bool hover = false;
        if (m.x > 840 && m.x < 1560 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          hover = true;
          if (Mouse::isButtonPressed(Mouse::Left))
          {
            selectedDevice = i;
          }
        }
        select.draw(drawRect(840, i * 21 + 100, 720, 20, Color(0, 0, 127 + (hover * 50) + ((i == selectedDevice) * 70))));
        select.draw(drawText(font, 865, i * 21 + 100, 18, MIDIDevices[i], Color::White, "l"));
        select.draw(drawSprite(midi, 841, i * 21 + 101, 2));
      }
    }
    else
    {
      select.draw(drawText(thiccfont, 1150, 120, 20, "No MIDI Devices Found", Color::White, "l"));
    }

    select.display();
  }
  return 0;
}

int displayPlayerScreen()
{
  uint64_t actualNoteCount = 0;
  RenderWindow window(VideoMode(1600, 1000), "Player", Style::Titlebar);
  cout << "Render start" << endl;
  window.setVerticalSyncEnabled(true);

  globalLength = midiData[midiData.size() - 1][0];
  float step = 1540.0 / (float)globalLength;

  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed)
      {
        window.close();
        stopAllNotes(midiOut);
        return 0;
      }
    }
    if (Keyboard::isKeyPressed(Keyboard::Escape))
    {
      window.close();
      return 0;
    }

    m = Mouse::getPosition(window);

    window.clear();

    window.draw(drawRect(10, 10, 1560, 250, Color(15, 15, 15)));
    window.draw(drawSprite(grid, 20, 20, 2));

    window.draw(drawText(font, 20, 210, 32, to_string((int)Tempo) + " BPM (" + to_string((int)Tempo * speeds[speedIndex]) + ")", Color::White, "l"));
    window.draw(drawText(font, 1200, 210, 32, to_string((int)fps) + "FPS", Color::White, "l"));
    window.draw(drawRect(10, 270, 1560, 20, Color(60, 60, 60)));
    int x = ((float)step * (float)MIDITime) + 10;
    window.draw(drawSprite(marker, x - 6, 270, 2));
    if (inside(x - 10, x + 10, 260, 280) || seekT)
    {
      if (Mouse::isButtonPressed(Mouse::Left))
      {
        if (!seekT)
          stopAllNotes(midiOut);
        playing = false;
        MIDITime = (float)(m.x - 10) / (float)step;
        while (midiData[globalIndex][0] < MIDITime)
          globalIndex++;
        while (midiData[globalIndex][0] > MIDITime)
          globalIndex--;
      }
      seekT = Mouse::isButtonPressed(Mouse::Left);
    }
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

    if (inside(20, 64, 310, 354) && !seekT)
    {
      stop.setColor(Color(160, 160, 160));
      play.setColor(Color(160, 160, 160));
      if (Mouse::isButtonPressed(Mouse::Left) && !playT)
        playing = !playing;
      playT = Mouse::isButtonPressed(Mouse::Left);
    }

    if (playing)
      window.draw(drawSprite(stop, 20, 310, 4));
    else
      window.draw(drawSprite(play, 20, 310, 4));

    play.setColor(Color(255, 255, 255));
    stop.setColor(Color(255, 255, 255));

    window.draw(drawSprite(speed, 80, 310, 4));
    window.draw(drawText(font, 80, 360, 32, speedNames[speedIndex], Color::White, "l"));
    x = (speedIndex * 40) + 126;
    window.draw(drawCircle(x, 332, 10, Color::Red));
    if (inside(80, 452, 310, 354) && !seekT)
    {
      if (Mouse::isButtonPressed(Mouse::Left))
      {
        int lowestIndex = 8;
        for (int i = 0; i < 8; i++)
        {
          if (abs(speedX[i] - m.x) < abs(speedX[lowestIndex] - m.x))
          {
            lowestIndex = i;
          }
        }
        speedIndex = lowestIndex;
      }
    }

    bool backHover = false;
    if (inside(10, 410, 930, 990))
    {
      backHover = true;
      if (Mouse::isButtonPressed(Mouse::Left))
      {
        return 1;
      }
    }
    window.draw(drawRect(10, 930, 400, 60, Color(100 + (backHover * 50), 0, 0)));
    window.draw(drawText(thiccfont, 210, 940, 32, "Back", Color::White, "c"));

    window.display();

    if (playing)
      MIDITime += (((double)Tempo * (double)PPQN) / 60.f) * (1.f / (double)fps) * (double)speeds[speedIndex];
    MIDITime = min(MIDITime, (uint64_t)globalLength);

    while (midiData[globalIndex][0] <= MIDITime && playing)
    {
      thirdytwo2eight(midiData[globalIndex][1]);
      if (byte1 < 16 && byte4 != 0xFF)
      { // note event
        if (byte4)
        {
          playNote(midiOut, byte1, byte2, byte3);
        }
        else
        {
          stopNote(midiOut, byte1, byte2);
        }
        activeNotes[byte1][byte2] = byte4;
      }
      else if (byte1 > 15 && byte4 != 0xFF)
      {
        switch (byte1)
        {
        case 16: // program change
          sendProgramChange(midiOut, byte2, byte3);
          break;
        }
      }
      else if (byte4 == 0xFF)
      { // tempo change
        Tempo = 60000000.f / (byte1 << 16 | byte2 << 8 | byte3);
      }
      globalIndex++;
    }

    fps = 0.96 / measure.restart().asSeconds();
    if (fps < 10.0)
      fps = 60.0;
  }
  stopAllNotes(midiOut);
  return 0;
}