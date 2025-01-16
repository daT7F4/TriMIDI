#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "./src/drawFunctions.hpp"
#include "./src/synthesis.hpp"

#define u8 uint8_t
#define u32 uint32_t
#define OFFSET 100

using namespace std;
namespace fs = std::filesystem;

float fps = 60.0;

vector<u8> MD;
vector<vector<uint64_t>> notes;          // timestamp, (channel | note | velocity | on/off)
vector<vector<uint64_t>> systemMessages; // timestamp, (value index, channel, value, 0)
vector<vector<uint64_t>> meta;           // timestamp, tempo

u8 header[8] = {0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06};
u8 trackHeader[4] = {0x4D, 0x54, 0x72, 0x6B};

bool activeNotes[16][128];

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

uint64_t rI;
uint64_t noteIndex, metaIndex, systemIndex;

uint16_t trackCount;
uint16_t PPQN;
u32 delta;
uint64_t trackTime;
bool report;
uint64_t MIDITime;
float Tempo;

u8 eventType;
u8 channel;

uint16_t eight2sixteen(u8 b1, u8 b2) { return b1 << 8 | b2; }
u32 eight2twentyfour(u8 b1, u8 b2, u8 b3) { return b1 << 16 | b2 << 8 | b3; }
u32 eight2thirtytwo(u8 b1, u8 b2, u8 b3, u8 b4)
{
  return b1 << 24 | b2 << 16 | b3 << 8 | b4;
}

void getDelta()
{
  delta = 0;
  while (MD[rI] > 127)
  {
    delta = delta << 7 | MD[rI] & 127;
    rI++;
  }
  delta = delta << 7 | MD[rI] & 127;
  rI++;
}

void getMeta()
{
  rI++;
  u32 tempo = 0;
  switch (MD[rI])
  {
  case 0x51:
    tempo = eight2twentyfour(MD[rI + 2], MD[rI + 3], MD[rI + 4]);
    meta.push_back({trackTime, tempo});
    break;
  }
  rI++;
  rI += MD[rI];
  rI++;
}

void getEvent()
{
  if ((MD[rI] & 0xF0) >> 4 > 7)
  {
    eventType = (MD[rI] & 0xF0) >> 4;
    channel = MD[rI] & 0x0F;
    rI++;
  }
  switch (eventType)
  {
  case 0b1000: // note off
    notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 0)});
    rI += 2;
    break;
  case 0b1001: // note on
    if (MD[rI + 1] == 0)
    {
      notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 0)});
    }
    else
    {
      notes.push_back({trackTime, eight2thirtytwo(channel, MD[rI], MD[rI + 1], 1)});
    }
    rI += 2;
    break;
  case 0b1010: // aftertouch
    rI += 2;
    break;
  case 0b1011: // control change
    rI += 2;
    break;
  case 0b1100: // program change
    systemMessages.push_back({trackTime, eight2thirtytwo(0, channel, MD[rI], 0)});
    rI += 1;
    break;
  case 0b1101: // channel pressure
    rI += 1;
    break;
  case 0b1110: // pitch wheel
    rI += 2;
    break;
  case 0b1111: // system exclusive
    while (MD[rI] != 0xF7)
    {
      rI++;
    }
    rI++;
    break;
  default:
    cerr << "Unknown event type 0x" << hex << uppercase << eventType << " at 0x" << hex
         << rI - 1 << endl;
    report = true;
  }
}

int main()
{
  initFont();

  vector<string> files;

  for (const auto &entry : fs::directory_iterator("./../"))
  {
    if (entry.is_regular_file())
    {
      std::string ext = entry.path().extension().string();
      if (ext == ".mid" || ext == ".midi")
      {
        files.push_back(entry.path().filename().string());
      }
    }
  }

  RenderWindow select(sf::VideoMode(800, 600), "Selection Screen");
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
        select.close();
    }
    select.clear();
    Vector2i m = Mouse::getPosition(select);

    bool startHover = false, exitHover = false;

    if (m.x > 590 && m.x < 790 && m.y > 10 && m.y < 40)
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
    select.draw(drawText(330, 5, 20, "v.1.0", Color::White, 1));

    select.draw(drawRect(590, 10, 200, 30, Color(0, 100 + (startHover * 50), 0)));
    select.draw(drawRect(590, 50, 200, 30, Color(100 + (exitHover * 50), 0, 0)));

    select.draw(drawText(690, 15, 15, "Start", Color::White, 0));
    select.draw(drawText(690, 55, 15, "Exit", Color::White, 0));

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
      select.draw(drawText(400, i * 21 + 100, 18, MIDIDevices[i], Color::White, 1));
    }

    select.display();
  }

  ifstream file("./../" + files[selectedFile], ios::binary);

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

  cout << "Done decoding" << endl;
  MD.clear();

  RtMidiOut midiOut;

  unsigned int nPorts = midiOut.getPortCount();
  if (nPorts == 0)
  {
    std::cerr << "No MIDI output ports available!" << std::endl;
    return 1;
  }

  midiOut.openPort(selectedDevice);

  Tempo = 60000000.0 / meta[0][1];

  RenderWindow window(sf::VideoMode(800, 600), "Player");
  window.setVerticalSyncEnabled(true);

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

    window.clear();

    window.draw(drawRect(10, 10, 780, 372, Color(15, 15, 15)));

    window.draw(drawText(10, 362, 16, to_string(Tempo) + " BPM", Color::White, 1));
    window.draw(drawText(10, 400, 30, to_string(fps) + "FPS", Color::White, 1));

    for (int i = 0; i < 16; i++)
    {
      for (int j = 0; j < 128; j++)
      {
        if (j % 12 == 0)
        {
          window.draw(drawRect(j * 6 + 18, 18, 5, 16 * 21, Color(100, 100, 100)));
        }
        if (activeNotes[i][j])
        {
          window.draw(drawRect(j * 6 + 18, i * 21 + 18, 5, 20, midiColors[i]));
        }
      }
    }

    window.display();

    MIDITime += ((Tempo * PPQN) / 60) * (1.0 / fps);

    while (notes[noteIndex][0] < MIDITime)
    {
      uint32_t value = notes[noteIndex][1];
      uint8_t byte1 = (value >> 24) & 0xFF;
      uint8_t byte2 = (value >> 16) & 0xFF;
      uint8_t byte3 = (value >> 8) & 0xFF;
      uint8_t byte4 = value & 0xFF;
      if (byte4)
      {
        playNote(midiOut, byte1, byte2, byte3);
        activeNotes[byte1][byte2] = true;
      }
      else
      {
        stopNote(midiOut, byte1, byte2);
        activeNotes[byte1][byte2] = false;
      }
      noteIndex++;
    }

    while (meta[metaIndex][0] < MIDITime && metaIndex < meta.size() - 1)
    {
      metaIndex++;
      Tempo = 60000000.0 / meta[metaIndex][1];
    }
    while (systemMessages[systemIndex][0] < MIDITime && systemIndex < systemMessages.size() - 1)
    {
      uint32_t value = systemMessages[systemIndex][1];
      uint8_t byte1 = (value >> 24) & 0xFF;
      uint8_t byte2 = (value >> 16) & 0xFF;
      uint8_t byte3 = (value >> 8) & 0xFF;
      uint8_t byte4 = value & 0xFF;
      switch (byte1)
      {
      case 0:
        sendProgramChange(midiOut, byte2, byte3);
        break;
      }
      systemIndex++;
    }
    currentTime = clock.getElapsedTime();
    fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
    previousTime = currentTime;
  }

  midiOut.closePort();

  return 0;
}