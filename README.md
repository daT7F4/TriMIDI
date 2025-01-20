# TriMIDI
MIDI Player in SFML

# How to use
Download the .zip and extract.

All the midi files should be in the "midi_files" directory which is created when building.

To compile and build it run this command from root.
```
make build
```
To run the compiled program do
```
make run
```

To remove the build do
```
make clean
```

Tested with external MIDI synth ONLY.

If you want to just compile the changed files do
```
make sb
```

### Dependencies you might need

Automatic installation can be done using the command
```
make install-deps
```

* libflac-dev
* libvorbis-dev
* libopenal-dev
* libopenal1
* libudev-dev
* libx11-dev
* libxrandr-dev
* libxcursor-dev
* libfreetype6-dev