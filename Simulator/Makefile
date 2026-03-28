##
## EPITECH PROJECT, 2026
## Pharmalgo
## File description:
## Makefile
##

TARGET = challenge
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC = main.cpp

LIB = libsim.a

all: $(TARGET)

$(TARGET): $(SRC) $(LIB)
	$(CXX) $(CXXFLAGS) $(SRC) $(LIB) -o $(TARGET)

clean:
	rm -f $(TARGET)

fclean: clean

re: fclean all
