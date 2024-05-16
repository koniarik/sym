

configure:
	cmake -Bbuild/

build: configure
	cmake --build build/
