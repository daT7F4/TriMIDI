#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "synthesis.hpp"
#include "drawFunctions.hpp"
#include "readFunctions.hpp"

RtMidiOut midiOut;

sf::Clock measure;
sf::Vector2i m;

vector<string> files;
uint64_t globalIndex, tempoIndex;
uint64_t globalLength;
uint16_t PPQN;
uint64_t MIDITime;
uint64_t lastMIDITime;
uint16_t trackCount;

bool renderingNotes[16][128];
bool mutedTracks[1024];

int selectedFile = -1;
int selectedDevice = -1;

bool seekT = false;
bool playT = false;
bool trackT = false;
bool transT = false;
bool playing = true;
bool internalSynth = false;

float Tempo;
double FPS = 60.0;

sf::Color midiColors[12] = {
    sf::Color(255, 0, 0),
    sf::Color(255, 127, 0),
    sf::Color(127, 127, 0),
    sf::Color(127, 255, 0),
    sf::Color(0, 255, 0),
    sf::Color(0, 255, 127),
    sf::Color(0, 127, 127),
    sf::Color(0, 127, 255),
    sf::Color(0, 0, 255),
    sf::Color(127, 0, 255),
    sf::Color(127, 0, 127),
    sf::Color(255, 0, 127)
};

float speeds[8] = {0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2};
string speedNames[8] = {"0.25x", "0.5x", "0.75x", "1x", "1.25x", "1.5x", "1.75x", "2x"};
int speedX[9] = {124, 164, 205, 247, 286, 326, 367, 407, 448};
int speedIndex = 3;

sf::Color brigthness(sf::Color in, float bright){
  return sf::Color(in.r * bright, in.g * bright, in.b * bright);
}

bool inside(int x1, int x2, int y1, int y2)
{
  return m.x > x1 && m.x < x2 && m.y > y1 && m.y < y2;
}

void stopAllNotes(RtMidiOut &midiOut)
{
  for (int i = 0; i < 128; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      stopNote(midiOut, j, i, 0);
      renderingNotes[j][i] = false;
      activeNotes[j][i] = false;
    }
  }
}

void stopAllSynthNotes(){
  for(int i = 0; i < 128; i++){
    for(int j = 0; j < 16; j++){
      stopSynthNote(i, j, 0);
      renderingNotes[j][i] = false;
      activeNotes[j][i] = false;
    }
  }
}

int displaySelectionScreen()
{
  font.loadFromFile("./assets/SourceCodePro-Light.ttf");
  thiccfont.loadFromFile("./assets/SourceCodePro-Black.ttf");
  if (!texture.loadFromFile("./assets/texture.png"))
    cerr << "Failed to load texture" << endl;

  Label name;
  name.x = 5;
  name.y = 5;
  name.size = 80;
  name.text = "TriMIDI";
  name.color = sf::Color::White;
  name.InitText(thiccfont);
  Label version;
  version.x = 330;
  version.y = 5;
  version.size = 20;
  version.text = "v.1.5.5";
  version.color = sf::Color::White;
  version.InitText(font);

  Button start;
  start.x = 1190;
  start.y = 10;
  start.w = 400;
  start.h = 30;
  start.LabelText = "Start";
  start.Locked = sf::Color(0, 100, 0);
  start.Open = sf::Color(0, 150, 0);
  start.Hover = sf::Color(0, 200, 0);
  start.mode = 0;
  start.centerAllign = true;
  Button exit;
  exit.x = 1190;
  exit.y = 50;
  exit.w = 400;
  exit.h = 30;
  exit.LabelText = "Exit";
  exit.Locked = sf::Color(100, 0, 0);
  exit.Open = sf::Color(150, 0, 0);
  exit.Hover = sf::Color(200, 0, 0);
  exit.mode = 1;
  exit.centerAllign = true;

  Button selection;
  selection.Locked = sf::Color(0, 0, 100);
  selection.Open = sf::Color(0, 0, 150);
  selection.Hover = sf::Color(0, 0, 200);
  selection.centerAllign = false;

  Label selectionError;
  selectionError.color = sf::Color::White;
  selectionError.size = 20;
  selectionError.allign = "c";

  Rectangle bar;
  bar.x = 5;
  bar.y = 90;
  bar.w = 1590;
  bar.h = 1;
  bar.color = sf::Color::White;
  bar.InitRect();

  sf::RenderWindow select(sf::VideoMode(1600, 1000), "Selection Screen", sf::Style::Titlebar);

  select.setFramerateLimit(60);

  listMidiDevices();

  while (select.isOpen())
  {
    sf::Event event;
    while (select.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        select.close();
        return 0;
      }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
    {
      select.close();
      return 0;
    }

    select.clear();

    if(select.hasFocus())
      m = sf::Mouse::getPosition(select);
    else
      m = sf::Vector2i(0,0);

    start.mode = 1;
    if (inside(1180, 1580, 10, 40) && selectedDevice != -1 && selectedFile != -1)
    {
      start.mode = 2;
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        select.close();
        return 1;
      }
    }
    else if (files.size() == 0 || MIDIDevices.size() == 0 || selectedDevice == -1 || selectedFile == -1)
    {
      start.mode = 0;
    }

    exit.mode = 1;
    if (inside(1180, 1580, 50, 80))
    {
      exit.mode = 2;
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        return 0;
      }
    }

    bar.DrawRect(select);

    name.DrawText(select);
    version.DrawText(select);

    start.InitButton();
    exit.InitButton();
    start.DrawButton(select);
    exit.DrawButton(select);

    if (files.size() > 0)
    {
      for (int i = 0; i < files.size(); i++)
      {
        selection.mode = 0;
        if (selectedFile == i)
          selection.mode = 1;
        if (inside(40, 800, i * 21 + 100, i * 21 + 120))
        {
          selection.mode = 2;
          if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            selectedFile = i;
        }
        selection.x = 40;
        selection.y = i * 21 + 100;
        selection.w = 760;
        selection.h = 20;
        selection.LabelText = files[i];
        selection.textOffset = 0;
        selection.InitButton();
        selection.DrawButton(select);
      }
    }
    else
    {
      selectionError.text = "No MIDI Files Found";
      selectionError.x = 400;
      selectionError.y = 120;
      selectionError.InitText(thiccfont);
      selectionError.DrawText(select);
    }

    if (MIDIDevices.size() > 0)
    {
      for (int i = 0; i < MIDIDevices.size(); i++)
      {
        selection.mode = 0;
        if (selectedDevice == i)
          selection.mode = 1;
        if (m.x > 840 && m.x < 1560 && m.y > i * 21 + 100 && m.y < i * 21 + 120)
        {
          selection.mode = 2;
          if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            selectedDevice = i;
        }
        selection.x = 840;
        selection.y = i * 21 + 100;
        selection.w = 760;
        selection.h = 20;
        selection.LabelText = MIDIDevices[i];
        selection.textOffset = 0;
        selection.InitButton();
        selection.DrawButton(select);
      }
    }
    else
    {
      selectionError.text = "No MIDI Devices Found";
      selectionError.x = 1150;
      selectionError.y = 120;
      selectionError.InitText(thiccfont);
      selectionError.DrawText(select);
    }

    select.display();
  }
  return 0;
}

int displayPlayerScreen()
{
  initSynth();

  Rectangle background;
  background.x = 10;
  background.y = 10;
  background.w = 1560;
  background.h = 250;
  background.color = sf::Color(15, 15, 15);
  background.InitRect();
  Sprite grid;
  grid.x = 20;
  grid.y = 20;
  grid.scale = 2;
  grid.InitSprite(sf::IntRect(0, 0, 769, 97));

  Label bpm;
  bpm.x = 20;
  bpm.y = 210;
  bpm.size = 32;
  bpm.color = sf::Color::White;
  bpm.allign = "l";
  Label fps;
  fps.x = 1200;
  fps.y = 210;
  fps.size = 32;
  fps.color = sf::Color::White;
  fps.allign = "l";

  Rectangle scrubBackground;
  scrubBackground.x = 10;
  scrubBackground.y = 270;
  scrubBackground.w = 1560;
  scrubBackground.h = 20;
  scrubBackground.color = sf::Color(60, 60, 60);
  scrubBackground.InitRect();

  Sprite marker;
  marker.scale = 2;

  Sprite play;
  play.x = 20;
  play.y = 310;
  play.scale = 4;
  play.InitSprite(sf::IntRect(0, 98, 11, 11));
  Sprite stop;
  stop.x = 20;
  stop.y = 310;
  stop.scale = 4;
  stop.InitSprite(sf::IntRect(12, 98, 11, 11));

  Rectangle note;
  note.w = 10;
  note.h = 10;

  Sprite speed;
  speed.x = 80;
  speed.y = 310;
  speed.scale = 4;
  speed.InitSprite(sf::IntRect(42, 98, 93, 11));
  Label speedLabel;
  speedLabel.x = 460;
  speedLabel.y = 310;
  speedLabel.size = 32;
  speedLabel.color = sf::Color::White;
  speedLabel.allign = "l";
  Circle speedSetting;
  speedSetting.color = sf::Color(255, 0, 0);
  speedSetting.y = 332;
  speedSetting.r = 10;

  Button back;
  back.x = 10;
  back.y = 930;
  back.w = 400;
  back.h = 60;
  back.LabelText = "Back";
  back.Open = sf::Color(200, 0, 0);
  back.Hover = sf::Color(255, 0, 0);
  back.mode = 1;
  back.centerAllign = true;
  back.InitButton();

  Button track;
  track.w = 150;
  track.h = 30;
  track.Locked = sf::Color(80, 80, 80);
  track.Open = sf::Color(140, 140, 140);
  track.Hover = sf::Color(200, 200, 200);
  track.centerAllign = true;

  Sprite plus;
  plus.x = 650;
  plus.y = 217;
  plus.scale = 3;
  plus.InitSprite(sf::IntRect(136, 98, 11, 11));
  Sprite minus;
  minus.x = 500;
  minus.y = 217;
  minus.scale = 3;
  minus.InitSprite(sf::IntRect(148, 98, 11, 11));
  Label transposeLabel;
  transposeLabel.x = 585;
  transposeLabel.y = 213;
  transposeLabel.allign = "c";
  transposeLabel.size = 32;
  transposeLabel.color = sf::Color::White;

  uint64_t actualNoteCount = 0;
  sf::RenderWindow window(sf::VideoMode(1600, 1000), "Player", sf::Style::Titlebar);
  cout << "Render start" << endl;
  window.setVerticalSyncEnabled(true);

  sixtyfour2thridytwo(midiData[midiData.size() - 1]);
  globalLength = int1;
  float step = 1540.0 / (float)globalLength;

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        window.close();
        stopAllNotes(midiOut);
        return 0;
      }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
    {
      window.close();
      return 0;
    }

    if(window.hasFocus())
      m = sf::Mouse::getPosition(window);
    else
      m = sf::Vector2i(0,0);

    window.clear();

    background.DrawRect(window);
    grid.DrawSprite(window);

    bpm.text = to_string((int)Tempo) + " BPM (" + to_string((int)((int)Tempo * speeds[speedIndex])) + ")";
    bpm.InitText(font);
    bpm.DrawText(window);

    fps.text = to_string((int)FPS) + "FPS";
    fps.InitText(font);
    fps.DrawText(window);

    scrubBackground.DrawRect(window);
    int x = ((float)step * (float)MIDITime) + 10;
    marker.x = x - 6;
    marker.y = 270;
    marker.InitSprite(sf::IntRect(24, 98, 7, 11));
    marker.DrawSprite(window);
    if (inside(x - 10, x + 10, 260, 280) || seekT)
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        if (!seekT){
          if(internalSynth)
            stopAllSynthNotes();
          else
            stopAllNotes(midiOut);
        }
        playing = false;
        MIDITime = (float)(m.x - 10) / (float)step;
        sixtyfour2thridytwo(midiData[globalIndex]);
        while (int1 < MIDITime)
        {
          globalIndex++;
          sixtyfour2thridytwo(midiData[globalIndex]);
        }
        while (int1 > MIDITime)
        {
          globalIndex--;
          sixtyfour2thridytwo(midiData[globalIndex]);
        }
        sixtyfour2thridytwo(tempo[tempoIndex]);
        while (int1 < MIDITime && tempoIndex < tempo.size() - 1)
        {
          tempoIndex++;
          sixtyfour2thridytwo(tempo[tempoIndex]);
        }
        while (int1 > MIDITime && tempoIndex > 0)
        {
          tempoIndex--;
          sixtyfour2thridytwo(tempo[tempoIndex]);
        }
      }
      seekT = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    }
    for (int i = 0; i < 16; i++)
    {
      for (int j = 0; j < 128; j++)
      {
        if (renderingNotes[i][j])
        {
          note.x = 22 + (j * 12);
          note.y = 22 + (i * 12);
          note.color = midiColors[tracks[i][j] % 12];
          note.color.a = 255 - ((renderingNotes[i][j] + activeNotes[i][j]) * 128);
          note.InitRect();
          note.DrawRect(window);
        }
      }
    }

    if (inside(20, 64, 310, 354) && !seekT)
    {
      stop.color = sf::Color(160, 160, 160);
      play.color = sf::Color(160, 160, 160);
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !playT)
        playing = !playing;
      playT = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    }

    if (playing)
    {
      stop.DrawSprite(window);
    }
    else
    {
      play.DrawSprite(window);
    }

    play.color = sf::Color(255, 255, 255);
    stop.color = sf::Color(255, 255, 255);

    speed.DrawSprite(window);

    speedLabel.text = speedNames[speedIndex];
    speedLabel.InitText(font);
    speedLabel.DrawText(window);

    speedSetting.x = speedX[speedIndex];
    speedSetting.InitCircle();
    speedSetting.DrawCircle(window);

    if (inside(80, 452, 310, 354) && !seekT)
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
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

    back.mode = 1;
    if (inside(10, 410, 930, 990))
    {
      back.mode = 2;
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
      {
        return 1;
      }
    }
    back.DrawButton(window);

    for (int i = 0; i < trackCount; i++)
    {
      track.x = ((i % 10) * 158) + 10;
      track.y = ((i / 10) * 35) + 400;
      track.LabelText = "Track #" + to_string(i);
      track.mode = 1;
      track.Hover = brigthness(midiColors[i % 12], 0.8);
      track.Open = brigthness(midiColors[i % 12], 0.6);
      track.Locked = brigthness(midiColors[i % 12], 0.5);
      if (mutedTracks[i] == true){
        track.mode = 0;
        track.LabelText = "L Track #" + to_string(i);
        track.InitButton();
      }
      if (inside(track.x, track.x + 150, track.y, track.y + 30))
      {
        track.mode = 2;
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !trackT)
          mutedTracks[i] = !mutedTracks[i];
        trackT = sf::Mouse::isButtonPressed(sf::Mouse::Left);
      }
      track.InitButton();
      track.DrawButton(window);
    }

    plus.DrawSprite(window);
    minus.DrawSprite(window);
    transposeLabel.text = to_string(transpose);
    if (transpose > -1)
      transposeLabel.text = "+" + to_string(transpose);
    transposeLabel.InitText(thiccfont);
    transposeLabel.DrawText(window);
    if (inside(500, 544, 210, 254))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !transT && transpose > -24)
      {
        stopAllNotes(midiOut);
        transpose--;
      }
      transT = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    }
    if (inside(650, 694, 210, 254))
    {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !transT && transpose < 24)
      {
        stopAllNotes(midiOut);
        transpose++;
      }
      transT = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    }

    window.display();

    if (playing)
      MIDITime += (((double)Tempo * (double)PPQN) / 60.f) * (1.f / (double)FPS) * (double)speeds[speedIndex];
    MIDITime = min(MIDITime, (uint64_t)globalLength);

    sixtyfour2thridytwo(midiData[globalIndex]);
    while (int1 < MIDITime && playing)
    {
      thirdytwo2eight(int2);
      uint8_t channel = (byte1 & 0xF0) >> 4;
      uint16_t trackNumber = getTrackNumber();
      if (byte3 != 128)
      { // note event
        if ((byte4 & 128) >> 7)
        {
          if (mutedTracks[trackNumber] == false)
          {
            if(internalSynth)
              playSynthNote(byte2, channel, byte3);
            else
              playNote(midiOut, channel, byte2, byte3);
            activeNotes[channel][byte2] = true;
          }
          tracks[channel][byte2] = trackNumber;
        }
        else
        {
          if(internalSynth)
            stopSynthNote(byte2, channel, 0);
          else 
            stopNote(midiOut, channel, byte2, 0);
          activeNotes[channel][byte2] = false;
        }
        renderingNotes[channel][byte2] = (byte4 & 128) >> 7;
      }
      else if (byte3 == 128)
      {
        if(internalSynth)
          setSynthProgramChange(channel, byte2, channel == 9);
        else 
          sendProgramChange(midiOut, channel, byte2);
      }
      globalIndex++;
      sixtyfour2thridytwo(midiData[globalIndex]);
    }

    sixtyfour2thridytwo(tempo[tempoIndex + 1]);
    while (int1 < MIDITime && tempoIndex < tempo.size() - 1)
    {
      tempoIndex++;
      sixtyfour2thridytwo(tempo[tempoIndex]);
    }
    sixtyfour2thridytwo(tempo[tempoIndex]);
    thirdytwo2eight(int2);

    Tempo = 60000000.0 / (float)eight2twentyfour(byte1, byte2, byte3);

    FPS = 0.96 / measure.restart().asSeconds();
    if (FPS < 10.0)
      FPS = 60.0;
  }
  return 0;
}
