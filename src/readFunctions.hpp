#include <iostream>

using namespace std;

#define BUFFER_SIZE 256
uint8_t buffer[BUFFER_SIZE];
uint8_t bufferIndex;

ifstream file;

vector<uint64_t> midiData; // (timestamp, data)
// if first byte is lower than 16 then data is (channel | note | velocity | on/off)
// if first byte is higher than 16 then data is (value index, channel, value, 0)
// if last byte is 0XFF then data is (tempo, tempo, tempo, 0xFF)

uint64_t rI;
uint64_t trackIndex;
uint64_t trackTime;
uint8_t eventType;
uint8_t channel;
uint8_t byte1, byte2, byte3, byte4;
uint32_t int1, int2;
bool report = false;

void readNextByte(ifstream &file, uint bytes){
  for(int i = 0; i < bytes; i++){
    buffer[bufferIndex++] = file.get();
    bufferIndex = bufferIndex % BUFFER_SIZE;
  }
}

uint8_t get(uint64_t byte){
  return buffer[byte % BUFFER_SIZE];
}

void initFile(string name){
  file.open(name, ios::binary);
}

uint64_t thirdytwo2sixtyfour(uint32_t int1, uint32_t int2){return (uint64_t)int1 << 32 | (uint64_t)int2;}
void sixtyfour2thridytwo(uint64_t val){int1 = val >> 32; int2 = val;}
uint16_t eight2sixteen(uint8_t b1, uint8_t b2) { return b1 << 8 | b2; }
uint32_t eight2twentyfour(uint8_t b1, uint8_t b2, uint8_t b3) { return b1 << 16 | b2 << 8 | b3; }
uint32_t eight2thirtytwo(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
  return b1 << 24 | b2 << 16 | b3 << 8 | b4;
}
void thirdytwo2eight(uint32_t value)
{
  byte1 = (value >> 24) & 0xFF;
  byte2 = (value >> 16) & 0xFF;
  byte3 = (value >> 8) & 0xFF;
  byte4 = value & 0xFF;
}
void getDelta()
{
  uint32_t delta = 0;
  while (get(rI) > 127)
  {
    delta = delta << 7 | (get(rI) & 127);
    rI++; readNextByte(file, 1);
  }
  delta = delta << 7 | (get(rI) & 127);
  rI++; readNextByte(file, 1);
  trackTime += delta;
}

void getMeta()
{
  rI++; readNextByte(file, 1);
  switch (get(rI))
  {
  case 0x51:
    midiData.push_back(thirdytwo2sixtyfour(trackTime, eight2thirtytwo(get(rI + 2), get(rI + 3), get(rI + 4), 0xFF)));
    break;
  }
  rI++; readNextByte(file, 1);
  int len = get(rI);
  rI += len; readNextByte(file, len);
  rI++; readNextByte(file, 1);
}

void getEvent()
{
  if ((get(rI) & 0xF0) >> 4 > 7)
  {
    eventType = (get(rI) & 0xF0) >> 4;
    channel = get(rI) & 0x0F;
    rI++; readNextByte(file, 1);
  }
  switch (eventType)
  {
  case 0b1000: // note off
    midiData.push_back(thirdytwo2sixtyfour(trackTime, eight2thirtytwo(channel, get(rI), get(rI + 1), 0)));
    rI += 2; readNextByte(file, 2);
    break;
  case 0b1001: // note on
    if (get(rI + 1) == 0)
    {
      midiData.push_back(thirdytwo2sixtyfour(trackTime, eight2thirtytwo(channel, get(rI), get(rI + 1), 0)));
    }
    else
    {
      midiData.push_back(thirdytwo2sixtyfour(trackTime, eight2thirtytwo(channel, get(rI), get(rI + 1), 1)));
    }
    rI += 2; readNextByte(file, 2);
    break;
  case 0b1010: // aftertouch
    rI += 2; readNextByte(file, 2);
    break;
  case 0b1011: // control change
    rI += 2; readNextByte(file, 2);
    break;
  case 0b1100: // program change
    midiData.push_back(thirdytwo2sixtyfour(trackTime, eight2thirtytwo(16, channel, get(rI), 0)));
    rI += 1; readNextByte(file, 1);
    break;
  case 0b1101: // channel pressure
    rI += 1; readNextByte(file, 1);
    break;
  case 0b1110: // pitch wheel
    rI += 2; readNextByte(file, 2);
    break;
  case 0b1111: // system exclusive
    while (get(rI) != 0xF7)
    {
      rI++; readNextByte(file, 1);
    }
    rI++; readNextByte(file, 1);
    break;
  default:
    cerr << "Unknown event type 0x" << hex << uppercase << eventType << " at 0x" << hex
         << rI - 1 << endl;
    report = true;
  }
}