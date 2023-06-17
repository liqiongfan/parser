target: run

build:
	re2c --no-generation-date -bci parser.re -o parser.cpp

run:
	rm -rf build && mkdir build && cd build && cmake .. && make && ./parser

