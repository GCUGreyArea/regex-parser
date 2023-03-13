NAME  = parser
CXSRC = $(wildcard src/*.cpp)
CSRC  = $(wildcard src/*.c)

TARGET = benchmark_$(NAME)

LIBDIR  = ../lib
LIB     = lib$(NAME).so
BUILD   = build
LIBDEP  = $(LIBDIR)/$(BUILD)/$(LIB)


OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

LIBRARIES = -lpthread -lre2 -lhs -lbenchmark -lbenchmark_main -l$(NAME)

CC     = cc
CXX    = c++
CFLAGS = -std=c11 -Wall -fsanitize=address -I$(LIBDIR)/src
CXXFLAGS = -std=c++17 -Wall -fsanitize=address -I$(LIBDIR)/src

all:  build $(TARGET) $(OBJ) $(LIBDEP)

$(TARGET) : build $(OBJ) $(LIBDEP)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BUILD)/$(TARGET) -l$(NAME) $(LIBRARIES) -l$(NAME) -L$(LIBDIR)/$(BUILD) -I$(LIBDIR)

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
