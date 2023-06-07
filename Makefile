build:
	re2c --no-generation-date -bci parser.re -o parser.cpp

run:
	mkdir build && cd build && cmake .. && make && ./parser

