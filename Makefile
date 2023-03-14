TARGET = parser



BUILD	   = build
TESTDIR    = test
BENCHDIR   = benchmark
LIBDIR     = lib
DOXYDIR	   = docs
BUILD      = build
DEP		   = dependancies
VERSION    = v1.13.0
DEP        = dependancies

TESTTARGET = $(TESTDIR)/$(BUILD)/test_$(TARGET)
BNCTARGET  = $(BENCHDIR)/$(BUILD)/benchmark_$(TARGET)
LIBTARGET  = $(LIBDIR)/$(BUILD)/lib$(TARGET).so
DOCTARGET  = $(DOXYDIR)/generated

SRC = $(shell find src -name '*.cpp')

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SRC))

INC = -I$(LIBDIR)/src  -I$(PDW)/$(DEP)/gtest/googletest/include

CC       = cc
CXX      = c++
CFLAGS   = -std=c11 -Wall -g -fsanitize=address $(INC)
CXXFLAGS = -std=c++17 -Wall -g -O1 -fsanitize=address $(INC)

ARGS =

PWD :=$(shell pwd)
UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)
	GLIB=$(PWD)/$(DEP)/gtest/build/lib/libgtest.dylib
endif
ifeq ($(UNAME),Linux)
	GLIB=$(PWD)/$(DEP)/gtest/build/lib/libgtest.so
endif

CMAKE_GTEST_BLD=cmake -DBUILD_SHARED_LIBS=ON \
				-DBUILD_GMOCK=ON \
				-DCMAKE_CXX_FLAGS=-std=c++17

all: $(GLIB) $(LIBTARGET) $(TESTTARGET) $(BNCTARGET) $(TARGET)

$(GLIB):
	rm -rf $(PWD)/$(DEP)/gtest
	git clone -b $(VERSION) https://github.com/google/googletest.git $(PWD)/$(DEP)/gtest
	mkdir -p  $(PWD)/$(DEP)/gtest/build
	cd  $(PWD)/$(DEP)/gtest/build && \
	    $(CMAKE_GTEST_BLD) .. -Dgtest_disable_pthreads=ON && \
		cmake --build .


$(TARGET) : build $(GLIB) $(LIBTARGET) $(TESTTARGET) $(BENCHTARGET) $(OBJ)
	@echo "GLIB is " $(GLIB)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BUILD)/$(TARGET) -lpthread -lhs -l$(TARGET) -I$(LIBDIR)

build :
	mkdir -p "$(BUILD)/src"

$(LIBTARGET) :
	cd $(LIBDIR) && make EXTRA="$(INC)"

$(TESTTARGET) : $(LIBTARGET)
	cd $(TESTDIR) && make EXTRA="$(INC)"

$(BNCTARGET) : $(LIB)
	cd $(BENCHDIR) && make EXTRA=$(INC)

$(DOCTARGET) :
	cd $(DOXYDIR) && make

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

clean:
	cd $(DOXYDIR) && make clean
	cd $(BENCHDIR) && make clean
	cd $(TESTDIR) && make clean
	cd $(LIBDIR) && make clean
	rm -rf $(BUILD)


install: $(TARGET)
	cp $(TARGET) $(INSTDIR)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

docs: $(DOCTARGET)
	cd $(DOXYDIR) && doxygen Doxyfile

test: $(TESTTARGET)
	cd $(LIBDIR) && make
	cd $(TESTDIR) && make
	cd $(TESTDIR) && make run

benchmark: $(BNCTARGET)
	cd $(LIBDIR) && make
	cd $(BENCHDIR) && make
	cd $(BENCHDIR) && make run

runtest:
	cd $(TESTDIR) && make run

runbench:
	cd $(BENCHDIR) && make run

run:
	./$(TARGET) $(ARGS)

project:
	cd $(LIBDIR) && make
	cd $(TESTDIR) && make
	cd $(BENCHDIR) && make
	make

help:
	@echo "\n\n"
	@echo "============================================================================================="
	@echo "Run: 'make' to build the project"
	@echo "Run: 'make test' to build and run unit tests"
	@echo "Run: 'make benchmark' to build and run unit tests - currently no benchmarks are implemented"
	@echo "Run: 'make runtest' to run the unit tests"
	@echo "Run: 'make runbench' to run the benchmarks - currently no benchmarks are implemented"
	@echo "Run: 'make doxygen' to build doxygen documentation\n"
	@echo "This project depends on:\n"
	@echo "\tGoogles test framework availible at https://github.com/google/googletest"
	@echo "\tGoogles benchmark framework availible at https://github.com/google/benchmark"
	@echo "\tcpp-yaml availible at https://github.com/jbeder/yaml-cpp"
	@echo "============================================================================================="
	@echo "\n\n"

