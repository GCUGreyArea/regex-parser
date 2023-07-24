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
PWD       := $(shell pwd)
LINK      := -lgtest -lpthread -lglog  

LIBRARIES = -lre2 -lhs
CFLAGS   = -std=c11 -Wall -g -fsanitize=address $(INC)
CXXFLAGS = -std=c++17 -fPIC -Wall -g -fsanitize=address $(INC)


CXXFLAGS = -std=c++17 -Wall -g -O1 -fsanitize=address -I$(LIBDIR)/src

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif


all: $(TARGET) $(OBJ) $(COBJ) $(LIBDEP)

$(TARGET) : build $(CXXOBJ) $(COBJ)
	$(CXX) $(CXXFLAGS) $(CXXOBJ) $(COBJ) $(LNK) -o $(BUILD)/$(TARGET) -l$(NAME) $(LIBRARIES) $(LINK)

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

