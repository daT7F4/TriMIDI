#include <RtMidi.h>

using namespace std;

vector<string> MIDIDevices;

bool activeNotes[16][128];
int8_t transpose;

void playNote(RtMidiOut &midiOut, int channel, int note, int velocity)
{
  if (channel != 9)
    note += transpose;
  if (note > -1 && note < 128)
  {
    vector<unsigned char> message;
    message.push_back(0x90 | channel);
    message.push_back(note);
    message.push_back(velocity);
    midiOut.sendMessage(&message);
  }
}

void stopNote(RtMidiOut &midiOut, int channel, int note, bool force)
{
  if (channel != 9)
    note += transpose;
  if (note > -1 && note < 128)
  {
    if (activeNotes[channel][note - (transpose * (channel != 9))] || force)
    {
      vector<unsigned char> message;
      message.push_back(0x80 | channel);
      message.push_back(note);
      message.push_back(0);
      midiOut.sendMessage(&message);
    }
  }
}

void sendProgramChange(RtMidiOut &midiOut, unsigned char channel, unsigned char programNumber)
{
  unsigned char status = 0xC0 | channel;
  vector<unsigned char> message = {status, programNumber};
  midiOut.sendMessage(&message);
}

void listMidiDevices()
{
  try
  {
    RtMidiOut midiOut;
    MIDIDevices.clear();
    unsigned int nOutputPorts = midiOut.getPortCount();
    for (unsigned int i = 0; i < nOutputPorts; ++i)
    {
      try
      {
        string portName = midiOut.getPortName(i);
        MIDIDevices.push_back(portName);
      }
      catch (RtMidiError &error)
      {
        cerr << "Error getting output port name: " << error.getMessage() << endl;
      }
    }
  }
  catch (RtMidiError &error)
  {
    cerr << "An RtMidi error occurred: " << error.getMessage() << endl;
  }
}
