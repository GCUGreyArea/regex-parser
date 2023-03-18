NAME  = parser
CXSRC = $(wildcard src/*.cpp)
CSRC  = $(wildcard src/*.c)

TARGET = benchmark_$(NAME)

LIBDIR  = ../lib
LIB     = lib$(NAME).so
BUILD   = build
LIBDEP  = $(LIBDIR)/$(BUILD)/$(LIB)


OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif


all:  build $(TARGET) $(OBJ) $(LIBDEP)

$(TARGET) : build $(OBJ) $(LIBDEP)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BUILD)/$(TARGET) -l$(NAME) $(LINK) $(LIBRARIES) $(LNK)

build:
	mkdir -p "$(BUILD)/src"


$(LIBDEP) :
	cd $(LIBDIR) && make

run: $(TARGET)
	./$(BUILD)/$(TARGET)

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
