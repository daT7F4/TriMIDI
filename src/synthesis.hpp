#include <RtMidi.h>

vector<string> MIDIDevices;

void playNote(RtMidiOut &midiOut, int channel, int note, int velocity) {

    std::vector<unsigned char> message;
    message.push_back(0x90 | channel); // Note On message for the channel
    message.push_back(note);                // MIDI note number
    message.push_back(velocity);            // Velocity
    midiOut.sendMessage(&message);
}

// Function to turn off a MIDI note on a specific channel
void stopNote(RtMidiOut &midiOut, int channel, int note) {
    std::vector<unsigned char> message;
    message.push_back(0x80 | channel); // Note Off message for the channel
    message.push_back(note);                // MIDI note number
    message.push_back(0);                   // Velocity (ignored for Note Off)
    midiOut.sendMessage(&message);
}

// Function to send a Program Change message
void sendProgramChange(RtMidiOut &midiOut, unsigned char channel, unsigned char programNumber) {
    unsigned char status = 0xC0 | channel;
    std::vector<unsigned char> message = {status, programNumber};
    midiOut.sendMessage(&message);
    std::cout << "Program changed to " << (int)programNumber << " on channel " << (int)(channel + 1) << std::endl;
}

void listMidiDevices() {
    try {
        RtMidiOut midiOut;

        // List output devices
        std::cout << "\nMIDI Output Devices:" << std::endl;
        unsigned int nOutputPorts = midiOut.getPortCount();
        for (unsigned int i = 0; i < nOutputPorts; ++i) {
            try {
                std::string portName = midiOut.getPortName(i);
                std::cout << i << ": " << portName << std::endl;
                MIDIDevices.push_back(portName);
            } catch (RtMidiError &error) {
                std::cerr << "Error getting output port name: " << error.getMessage() << std::endl;
            }
        }
    } catch (RtMidiError &error) {
        std::cerr << "An RtMidi error occurred: " << error.getMessage() << std::endl;
    }
}