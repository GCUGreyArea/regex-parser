
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
HDRINST = /usr/local/include

OBJ    := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif

all: $(TARGET)

$(TARGET) : build $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -shared  -o $(BUILD)/$(TARGET) -l$(NAME) -lpthread -lre2 -lyaml-cpp $(LINK) $(LNK)

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
