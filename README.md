# TriMIDI
MIDI Player in SFML

# How to use
Download the .zip and extract.

All the midi files should be in the "midi_files" directory which is created when building.

To compile and build it do these commands from the root directory.
```
make install-deps
make build
```
To run it do
```
make run
```

To clean it up do
```
make clean
```

Tested with external MIDI synth ONLY.

If you want to just compile the changed files do
```
make sb
```

### Dependencies you might need (will be installed using the make command)

* libflac-dev
* libvorbis-dev
* libopenal-dev
* libopenal1
* libudev-dev
* libx11-dev
* libxrandr-dev
* libxcursor-dev
* libfreetype6-dev
