CPPFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb -O2

hw2 : hw2.cpp
	g++ $(CPPFLAGS) $< -o $@

all : hw2
	@echo "Made it all!"