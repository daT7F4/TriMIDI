

all: build

build:
	mkdir build && cd build && cmake ..

clean:
	rm -rf build

install-deps:
	sudo apt-get install libflac-dev libvorbis-dev libopenal-dev libopenal1 libudev-dev
       
