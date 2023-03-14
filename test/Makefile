NAME	= parser
TARGET  = test_$(NAME)

CXXSRC = $(wildcard src/*.cpp)
CSRC   = $(wildcard src/*.c)

LIB     = lib$(NAME).so
BUILD   = build
LIBDIR  = ../lib
LIBDEP  = $(LIBDIR)/$(BUILD)/$(LIB)

CXXOBJ    := $(patsubst %.cpp,$(BUILD)/%.o,$(CXXSRC))
COBJ      := $(patsubst %.c,$(BUILD)/%.o,$(CSRC))
PWD=$(shell pwd)

INC    = -I$(LIBDIR)/src -I  $(PWD)/../dependancies/gtest/googletest/include
LIB    = -L../dependancies/gtest/build/lib -rpath $(PWD)/../dependancies/gtest/build/lib
CC     = cc
CXX    = c++
CFLAGS = -g -std=c11 -Wall -fsanitize=address 
CXXFLAGS = -g -std=c++17 -Wall -O1 -fsanitize=address $(INC)

all: $(TARGET) $(OBJ) $(COBJ) $(LIBDEP)

$(TARGET) : build $(CXXOBJ) $(COBJ)
	$(CXX) $(CXXFLAGS) $(CXXOBJ) $(COBJ) -o $(BUILD)/$(TARGET) $(LIB) -l$(NAME) -lgtest -lpthread -lglog -lframework -lyaml-cpp

build:
	mkdir -p "$(BUILD)/src"

$(LIBDEP) :
	cd $(LIBDIR) && make

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

run:
	./$(BUILD)/$(TARGET)

clean:
	rm -rf $(BUILD)

