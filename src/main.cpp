#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "windows.hpp"

using namespace std;
namespace fs = filesystem;

uint8_t header[8] = {0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06};
uint8_t trackHeader[4] = {0x4D, 0x54, 0x72, 0x6B};

int playingNotes;

uint16_t trackCount;
uint64_t totalSize;

int main()
{

  for (const auto &entry : fs::directory_iterator("./midi_files"))
  {
    if (entry.is_regular_file())
    {
      string ext = entry.path().extension().string();
      if (ext == ".mid" || ext == ".midi")
      {
        files.push_back(entry.path().filename().string());
      }
    }
  }

  while (true)
  {
    if (displaySelectionScreen() == 0)
      return 0;

    file.open("./midi_files/" + files[selectedFile], ios::binary);

    if (!file.is_open())
    {
      cerr << "Error: Could not open file.\n";
      return 1;
    }

    char byte;
    midiData.clear();
    MIDITime = 0;
    globalIndex = 0;
    lastMIDITime = 0;
    for (int i = 0; i < 128; i++)
    {
      for (int j = 0; j < 16; j++)
      {
        activeNotes[j][i] = false;
      }
    }
    readNextByte(file, BUFFER_SIZE);

    bufferIndex = 0;
    rI = 0;

    // check header
    for (rI = 0; rI < 8; rI++)
    {
      if (get(rI) != header[rI])
      {
        cerr << "Invalid header" << endl;
        cerr << "Expected 0x" << hex << uppercase << get(rI) << endl;
        cerr << "Got 0x" << hex << uppercase << get(rI) << endl;
        return 1;
      }
    }
    readNextByte(file, 8);
    rI += 2; readNextByte(file, 2);
    trackCount = eight2sixteen(get(rI), get(rI + 1));
    cout << "Track count: " << trackCount << endl;
    rI += 2; readNextByte(file, 2);
    PPQN = eight2sixteen(get(rI), get(rI + 1));
    cout << "PPQN: " << PPQN << endl;
    rI += 2; readNextByte(file, 2);
    for (int track = 0; track < trackCount; track++)
    {
      for (int i = 0; i < 4; i++)
      {
        if (get((rI + i) % BUFFER_SIZE) != trackHeader[i])
        {
          cerr << "Invalid track header at 0x" << hex << uppercase << rI + i << endl;
          cerr << "Expected 0x" << hex << uppercase << trackHeader[i] << endl;
          cerr << "Got 0x" << hex << get((rI + i) % BUFFER_SIZE) << endl;
          return 1;
        }
      }
      rI += 4; readNextByte(file, 4);
      uint32_t trackLenght =
          eight2thirtytwo(get(rI), get(rI + 1), get(rI + 2), get(rI + 3));
      cout << "Track #" << track << ", Size: " << trackLenght << " bytes" << endl;
      totalSize += trackLenght;
      rI += 4; readNextByte(file, 4);
      uint64_t end = rI + trackLenght;
      trackTime = 0;
      uint64_t lastIndex = 0;
      while (rI < end)
      {
        getDelta();
        if (get(rI) == 0xFF)
          getMeta();
        else
          getEvent();
        if (report)
          return 1;
      }
    }

   std::sort(midiData.begin(), midiData.end(), [](const uint64_t& a, const uint64_t& b) {
        return (a >> 32) < (b >> 32); // Compare upper 32 bits
    });
    midiData.shrink_to_fit();

    cout << "Done decoding " << totalSize << " bytes" << endl;
    cout << "Total allocated size is " << midiData.size() * 8 << " bytes" << endl;

    unsigned int nPorts = midiOut.getPortCount();

    midiOut.openPort(selectedDevice);

    while (true)
    {
      sixtyfour2thridytwo(midiData[globalIndex]);
      thirdytwo2eight(int2);
      if (byte4 == 0xFF)
      {
        Tempo = 60000000.0 / eight2twentyfour(byte1, byte2, byte3);
        break;
      }
      globalIndex++;
    }
    globalIndex = 0;

    file.close();

    if (displayPlayerScreen() == 0)
      return 0;

    stopAllNotes(midiOut);

    midiOut.closePort();
  }

  return 0;
}
