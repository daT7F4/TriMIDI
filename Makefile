

all: build

build:
	mkdir build && mkdir midi_files && cd build && cmake .. && cmake --build . -j8

sb:
	cmake --build ./build -j8

run:
	./build/main

clean:
	rm -rf build

install-deps:
	sudo apt-get install libflac-dev libvorbis-dev libopenal-dev libopenal1 libudev-dev libx11-dev libxrandr-dev libxcursor-dev libfreetype6-dev
       
