

all: build

build:
	mkdir build && cd build && cmake .. && cmake --build .

clean:
	rm -rf build

install-deps:
	sudo apt-get install libflac-dev libvorbis-dev libopenal-dev libopenal1 libudev-dev libx11-dev libxrandr-dev libxcursor-dev libfreetype6-dev
       
