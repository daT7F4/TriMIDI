#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "./src/drawFunctions.hpp"
#include "./src/synthesis.hpp"
#include "./src/readFunctions.hpp"

#define u8 uint8_t
#define u32 uint32_t
#define OFFSET 100

using namespace std;
namespace fs = std::filesystem;

float fps = 60.0;

u8 header[8] = {0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06};
u8 trackHeader[4] = {0x4D, 0x54, 0x72, 0x6B};

bool activeNotes[16][128];
int playingNotes;
int globalLength;

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

uint64_t noteIndex, metaIndex, systemIndex;
bool seekT = false;
bool playT = false;
bool playing = true;

uint16_t trackCount;
uint16_t PPQN;
uint64_t MIDITime;
uint64_t lastMIDITime;
float Tempo;

RtMidiOut midiOut;

void updateNotes(bool reverse)
{
  thirdytwo2eight(notes[noteIndex][1]);
  if (byte4)
  {
    playNote(midiOut, byte1, byte2, byte3);
    if (reverse)
      playingNotes--;
    if (!reverse)
      playingNotes++;
    activeNotes[byte1][byte2] = !reverse;
  }
  else
  {
    stopNote(midiOut, byte1, byte2);
    if (!reverse)
      playingNotes--;
    if (reverse)
      playingNotes++;
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

int main()
{
  initFont();
  loadSprites();

  vector<string> files;

  for (const auto &entry : fs::directory_iterator("./midi_files"))
  {
    if (entry.is_regular_file())
    {
      std::string ext = entry.path().extension().string();
      if (ext == ".mid" || ext == ".midi")
      {
        files.push_back(entry.path().filename().string().substr(0, 20));
      }
    }
  }

  ContextSettings settings;
  settings.antialiasingLevel = 0;
  RenderWindow select(sf::VideoMode(800, 600), "Selection Screen", Style::Default, settings);
  select.setFramerateLimit(60);

  int selectedFile = -1;
  int selectedDevice = -1;

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
    select.draw(drawText(330, 5, 20, "v.1.0.1", Color::White, 1));

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

  ifstream file(files[selectedFile], ios::binary);

  if (!file.is_open())
  {
    cerr << "Error: Could not open file.\n";
    return 1;
  }

  char byte;
  while (file.get(byte))
  {
    MD.push_back(static_cast<u8>(byte));
  }

  file.close();

  // check header
  for (rI = 0; rI < 8; rI++)
  {
    if (MD[rI] != header[rI])
    {
      cerr << "Invalid header" << endl;
      cerr << "Expected 0x" << hex << uppercase << header[rI] << endl;
      cerr << "Got 0x" << hex << uppercase << MD[rI] << endl;
      return 1;
    }
  }
  rI += 2;
  trackCount = eight2sixteen(MD[rI], MD[rI + 1]);
  cout << "Track count: " << trackCount << endl;
  rI += 2;
  PPQN = eight2sixteen(MD[rI], MD[rI + 1]);
  cout << "PPQN: " << PPQN << endl;
  rI += 2;

  for (int track = 0; track < trackCount; track++)
  {
    for (int i = 0; i < 4; i++)
    {
      if (MD[rI + i] != trackHeader[i])
      {
        cerr << "Invalid track header at 0x" << hex << uppercase << rI + i << endl;
        cerr << "Expected 0x" << hex << uppercase << trackHeader[i] << endl;
        cerr << "Got 0x" << hex << MD[rI + i] << endl;
        return 1;
      }
    }
    rI += 4;
    u32 trackLenght =
        eight2thirtytwo(MD[rI], MD[rI + 1], MD[rI + 2], MD[rI + 3]);
    cout << "Track #" << track << " size: " << trackLenght << " Bytes" << endl;
    rI += 4;
    uint64_t end = rI + trackLenght;
    trackTime = 0;
    while (rI < end)
    {
      getDelta();
      trackTime += delta;
      if (MD[rI] == 0xFF)
      { // meta event
        getMeta();
      }
      else
      {
        getEvent();
      }
      if (report)
        return 1;
    }
  }

  sort(notes.begin(), notes.end(), [](const vector<uint64_t> &a, const vector<uint64_t> &b)
       { return a[0] < b[0]; });
  sort(systemMessages.begin(), systemMessages.end(), [](const vector<uint64_t> &a, const vector<uint64_t> &b)
       { return a[0] < b[0]; });
  sort(meta.begin(), meta.end(), [](const vector<uint64_t> &a, const vector<uint64_t> &b)
       { return a[0] < b[0]; });

  globalLength = notes[notes.size() - 1][0];
  float step = 770.0 / (float)globalLength;

  cout << "Done decoding" << endl;
  MD.clear();

  unsigned int nPorts = midiOut.getPortCount();

  midiOut.openPort(selectedDevice);

  Tempo = 60000000.0 / meta[0][1];

  RenderWindow window(sf::VideoMode(800, 600), "Player");
  window.setFramerateLimit(60);

  sf::Clock clock = Clock();
  sf::Time previousTime = clock.getElapsedTime();
  sf::Time currentTime;

  while (window.isOpen())
  {
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed)
        window.close();
    }

    Vector2i m = Mouse::getPosition(window);

    window.clear();

    window.draw(drawRect(5, 5, 780, 125, Color(15, 15, 15)));
    window.draw(drawSprite(grid, 10, 10, 1));

    window.draw(drawText(10, 105, 16, to_string(Tempo) + " BPM", Color::White, 1));
    window.draw(drawText(200, 105, 16, "Playing notes: " + to_string(playingNotes), Color::White, 1));
    window.draw(drawRect(10, 130, 780, 10, Color(15, 15, 15)));
    int x = ((float)step * (float)MIDITime) + 10;
    window.draw(drawSprite(marker, x - 3, 130, 1));
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

    if(playing)
      window.draw(drawSprite(play, 10, 150, 2));
    else
      window.draw(drawSprite(stop, 10, 150, 2));

    if(m.x > 10 && m.x < 30 && m.y > 150 && m.y < 170){
      if (Mouse::isButtonPressed(sf::Mouse::Left) && !playT){
        playing = !playing;
        playT = true;
      }
      if(!Mouse::isButtonPressed(sf::Mouse::Left))
        playT = false;
    }

    window.display();

    if(playing)
      MIDITime += (((float)Tempo * (float)PPQN) / 60.0) * (1.0 / 60.0);

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

  stopAllNotes(midiOut);

  midiOut.closePort();

  return 0;
}
