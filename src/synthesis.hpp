#include <RtMidi.h>
#include <fluidsynth.h>

using namespace std;

vector<string> MIDIDevices;

bool activeNotes[16][128];
uint8_t tracks[16][128];
int8_t transpose;

fluid_settings_t *settings;
fluid_synth_t *synth;
fluid_audio_driver_t *adriver;
int sfont_id;

void initSynth()
{
  settings = new_fluid_settings();
  if (!settings)
  {
    cerr << "Failed to create FluidSynth settings" << endl;
    return;
  }

  synth = new_fluid_synth(settings);
  if (!synth)
  {
    cerr << "Failed to create FluidSynth synth" << endl;
    delete_fluid_settings(settings);
    return;
  }

  adriver = new_fluid_audio_driver(settings, synth);
  if (!adriver)
  {
    cerr << "Failed to create FluidSynth audio driver" << endl;
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    return;
  }

  const char *soundfont_path = "./assets/weedsgm3.sf2";
  sfont_id = fluid_synth_sfload(synth, soundfont_path, 1);
  if (sfont_id == -1)
  {
    cerr << "Failed to load SoundFont: " << soundfont_path << endl;
  }
  cout << "Loaded SoundFont: " << soundfont_path << endl;
}

void playSynthNote(int note, int channel, int velocity)
{
  if (channel != 9)
    note += transpose;
  if (note > -1 && note < 128)
    fluid_synth_noteon(synth, channel, note, velocity);
}

void stopSynthNote(int note, int channel, bool force)
{
  if (channel != 9)
    note += transpose;
  if ((note > -1 && note < 128) && (activeNotes[channel][note - (transpose * (channel != 9))] || force))
    fluid_synth_noteoff(synth, channel, note);
}

void setSynthProgramChange(int channel, int program, bool drums)
{
  fluid_synth_program_select(synth, channel, sfont_id, drums * 128, program);
}

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
    MIDIDevices.push_back("Fluidsynth (Software Synth)");
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
