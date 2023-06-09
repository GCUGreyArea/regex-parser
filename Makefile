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

PWD :=$(shell pwd)
UNAME := $(shell uname)

SRC = $(shell find src -name '*.cpp')

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SRC))

INC = -I$(PWD)/$(LIBDIR)/src \
      -I$(PWD)/$(DEP)/bench/include \
	  -I$(PWD)/$(DEP)/gtest/googletest/include \
      -I$(PWD)/$(DEP)/re2 \
	  -I$(PWD)/$(DEP)/yaml-cpp/include \
	  -I$(PWD)/$(DEP)/json/include \
	  -I$(PWD)/$(DEP)/hyperscan/src

LNK = -L$(PWD)/lib/build \
	  -L$(PWD)/dependancies/bench/build/src \
      -L$(PWD)/dependancies/yaml-cpp/build \
	  -L$(PWD)/dependancies/gtest/build/lib \
	  -L$(PWD)/dependancies/re2/build_dir \
	  -L$(PWD)/dependancies/hyperscan/build/lib 

ifeq ($(UNAME),Darwin)
	LNK += -Wl,-rpath,$(PWD)/dependancies/bench/build/src,-rpath,$(PWD)/dependancies/yaml-cpp/build,-rpath,$(PWD)/lib/build,-rpath,$(PWD)/dependancies/re2/build_dir,-rpath,$(PWD)/dependancies/hyperscan/build/lib,-rpath,$(PWD)/dependancies/gtest/build/lib
endif
ifeq ($(UNAME),Linux)
	LNK += -Wl,-rpath,$(PWD)/dependancies/bench/build/src,-rpath,$(PWD)/dependancies/yaml-cpp/build,-rpath,$(PWD)/lib/build,-rpath,$(PWD)/dependancies/re2/build_dir,-rpath,$(PWD)/dependancies/hyperscan/build/lib
endif
    
LIBRARIES = -lbenchmark -lbenchmark_main -lyaml-cpp -lre2 -lgtest
CFLAGS   = -std=c11 -Wall -g -fsanitize=address $(INC)
CXXFLAGS = -std=c++17 -fPIC -Wall -g -O1 -fsanitize=address $(INC)

export CXXFLAGS
export CFLAGS
export LNK
export LIBRARIES

ARGS =

all: bench json yaml-cpp hyperscan re2 gtest $(LIBTARGET) $(TESTTARGET) $(BNCTARGET) $(TARGET)

RE2_URL:=https://github.com/google/re2.git
RE2_DEP:=dependancies/re2/build_dir/CMakeCache.txt

re2: $(RE2_DEP)

$(RE2_DEP): 
	rm -rf $(PWD)/$(DEP)/re2
	git clone -b main https://github.com/google/re2.git $(PWD)/$(DEP)/re2
	mkdir -p $(PWD)/$(DEP)/re2/build_dir
	cd $(PWD)/$(DEP)/re2/build_dir && cmake -DBUILD_SHARED_LIBS=ON .. && cmake --build .  
	
YAML_URL:=https://github.com/jbeder/yaml-cpp.git
YAML_DEP:= $(PWD)/$(DEP)/yaml-cpp/build/CMakeCache.txt

yaml-cpp: $(YAML_DEP)

$(YAML_DEP):
	rm -rf $(PWD)/$(DEP)/yaml-cpp
	git clone -b master $(YAML_URL) $(PWD)/$(DEP)/yaml-cpp
	mkdir -p $(PWD)/$(DEP)/yaml-cpp/build
	cd $(PWD)/$(DEP)/yaml-cpp/build && cmake -DBUILD_SHARED_LIBS=ON .. && cmake --build .


JSON_URL:=https://github.com/nlohmann/json.git
JSON_DEP:=$(PWD)/$(DEP)/json/include/nlohmann/json.hpp

json: $(JSON_DEP)

$(JSON_DEP):
	git clone $(JSON_URL) $(PWD)/$(DEP)/json


BENCH_URL:=https://github.com/google/benchmark.git
BENCH_DEP:=$(PWD)/$(DEP)/bench/build/CMakeCache.txt

bench: $(BENCH_DEP)

$(BENCH_DEP):
	rm -rf $(PWD)/$(DEP)/bench
	git clone $(BENCH_URL) $(PWD)/$(DEP)/bench
	mkdir -p $(PWD)/$(DEP)/bench/build
	cd $(PWD)/$(DEP)/bench/build && cmake -DBUILD_SHARED_LIBS=ON -DBENCHMARK_ENABLE_TESTING=OFF .. && cmake --build .


GLIB=$(PWD)/$(DEP)/gtest/build/CMakeCache.txt
CMAKE_GTEST_BLD=cmake -DBUILD_SHARED_LIBS=ON \
				-DBUILD_GMOCK=ON \
				-DCMAKE_CXX_FLAGS=-std=c++17

gtest: $(GLIB)

$(GLIB):
	rm -rf $(PWD)/$(DEP)/gtest
	git clone -b $(VERSION) https://github.com/google/googletest.git $(PWD)/$(DEP)/gtest
	mkdir -p $(PWD)/$(DEP)/gtest/build
	cd  $(PWD)/$(DEP)/gtest/build && \
	    $(CMAKE_GTEST_BLD) .. -Dgtest_disable_pthreads=ON && \
		cmake --build .

# Hyperscan needs boost and ragel to be installed
HYPERSCAN_DEP:=$(PWD)/dependancies/hyperscan/build/CMakeCache.txt
hyperscan: $(HYPERSCAN_DEP)

$(HYPERSCAN_DEP):
	git clone https://github.com/intel/hyperscan.git $(PWD)/$(DEP)/hyperscan
	mkdir -p $(PWD)/$(DEP)/hyperscan/build
	cd $(PWD)/$(DEP)/hyperscan/build && cmake -DBUILD_SHARED_LIBS=ON .. && cmake --build .

$(TARGET) : hyperscan json yaml-cpp re2 build gtest $(LIBTARGET) $(TESTTARGET) $(BENCHTARGET) $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BUILD)/$(TARGET) $(LNK) -lpthread -lhs -l$(TARGET)

build :
	mkdir -p "$(BUILD)/src"

$(LIBTARGET) :
	cd $(LIBDIR) && make 

$(TESTTARGET) : $(LIBTARGET)
	cd $(TESTDIR) && make

$(BNCTARGET) : $(LIB)
	cd $(BENCHDIR) && make

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

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

docs: $(DOCTARGET)
	cd $(DOXYDIR) && doxygen Doxyfile

test: $(TESTTARGET)
	cd $(LIBDIR) && make
	cd $(TESTDIR) && make

benchmark: $(BNCTARGET)
	cd $(LIBDIR) && make
	cd $(BENCHDIR) && make

project: bench json yaml-cpp hyperscan re2 gtest $(LIBTARGET) $(TESTTARGET) $(BNCTARGET) $(TARGET)

help:
	@echo "\n\n"
	@echo "============================================================================================="
	@echo "Run: 'make' to build the project"
	@echo "Run: 'make test' to build unit tests"
	@echo "Run: 'make benchmark' to build benchmarks - currently no benchmarks are implemented"
	@echo "Run: 'make doxygen' to build doxygen documentation"
	@echo "This project depends on:"
	@echo "\tGoogles test framework availible at https://github.com/google/googletest"
	@echo "\tGoogles benchmark framework availible at https://github.com/google/benchmark"
	@echo "\tyaml-cpp availible at https://github.com/jbeder/yaml-cpp"
	@echo "\thyperscan availible at https://github.com/intel/hyperscan"
	@echo "\tGoogle RE2 availible at https://github.com/google/re2"
	@echo "\tnlohman json parser availible at https://github.com/nlohmann/json"
	@echo "Build tools"
	@echo "\tcmake see https://cmake.org/"
	@echo "\tdoxygen see https://www.doxygen.nl/"
	@echo "Utilities"
	@echo "\tjq availible at https://github.com/stedolan/jq"
	@echo "============================================================================================="
	@echo "\n\n"

