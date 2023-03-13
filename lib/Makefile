
NAME  = parser

CXSRC = $(shell find src -name *.cpp)
CSRC  = $(shell find src -name *.c)
UNAME = $(shell uname)

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

BUILD = build
LIBDIR = .

YAML=$(BUILD)/src/YAML
EXCEPTION=$(BUILD)/src/Exception
OUTPUT=$(BUILD)/src/Output
FRAMEWORK=$(BUILD)/src/Framework


TARGET  = lib$(NAME).so
LIBDIR  =  .
INSTALL = /usr/local/lib
HDRINST = /usr/local/include`

OBJ    := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
CFLAGS   = -std=c11 -Wall -g -fsanitize=address -I$(LIBDIR)/src
CXXFLAGS = -std=c++17 -Wall -g -fsanitize=address -I$(LIBDIR)/src
else
CC     = gcc
CXX    = g++
CFLAGS   = -std=c11 -Wall -fPIC -fsanitize=address -I$(LIBDIR)/src
CXXFLAGS = -std=c++17 -Wall -fPIC -fsanitize=address -I$(LIBDIR)/src
endif


all: $(TARGET)

$(TARGET) : build $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -shared  -o $(BUILD)/$(TARGET) -lpthread -lframework -lre2 -lyaml-cpp
	make copy

build :
	mkdir -p $(YAML)
	mkdir -p $(EXCEPTION)
	mkdir -p $(OUTPUT)
	mkdir -p $(FRAMEWORK)

copy:
	cp $(BUILD)/$(TARGET) $(INSTALL)

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)
