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

INC = -I$(PWD)/$(LIBDIR)/src \
      -I$(PWD)/$(DEP)/bench/include \
	  -I$(PWD)/$(DEP)/gtest/include \
      -I$(PWD)/$(DEP)/re2 \
	  -I$(PWD)/$(DEP)/yaml-cpp/include \
	  -I$(PWD)/$(DEP)/json/include \
	  -I$(PWD)/$(DEP)/hyperscan/src

LNK = -L$(PWD)/lib/build \
	  -L$(PWD)/dependancies/bench/build/src \
      -L$(PWD)/dependancies/yaml-cpp/build \
	  -L$(PWD)/dependancies/gtest/build \
	  -L$(PWD)/dependancies/re2/build \
	  -L$(PWD)/dependancies/hyperscan/build/lib \
	  -Wl,-rpath,$(PWD)/dependancies/bench/build/src,-rpath,$(PWD)/dependancies/yaml-cpp/build,-rpath,$(PWD)/lib/build,-rpath,$(PWD)/dependancies/re2/build,-rpath,$(PWD)/dependancies/hyperscan/build/lib 
    
LIBRARIES = -lbenchmark -lbenchmark_main -lyaml-cpp -lre2 -lgtest
CFLAGS   = -std=c11 -Wall -g -fsanitize=address $(INC)
CXXFLAGS = -std=c++17 -fPIC -Wall -g -O1 -fsanitize=address $(INC)

export CXXFLAGS
export CFLAGS
export LNK
export LIBRARIES

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

all: bench json yaml-cpp re2 $(LIBTARGET) $(TESTTARGET) $(BNCTARGET) $(TARGET)

RE2_URL:=https://github.com/google/re2.git
RE2_DEP:=dependancies/re2/build/libre2.so

re2: $(RE2_DEP)

$(RE2_DEP): 
	rm -rf $(PWD)/$(DEP)/re2
	git clone -b main https://github.com/google/re2.git $(PWD)/$(DEP)/re2
	mkdir -p $(PWD)/$(DEP)/re2/build
	cd $(PWD)/$(DEP)/re2/build && cmake -DBUILD_SHARED_LIBS=ON .. && cmake --build .  
	
YAML_URL:=https://github.com/jbeder/yaml-cpp.git
YAML_DEP:= $(PWD)/$(DEP)/yaml-cpp/build/libyaml-cpp.so

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
BENCH_DEP:=$(PWD)/$(DEP)/bench/build/src/libbenchmark.so

bench: $(BENCH_DEP)

$(BENCH_DEP):
	rm -rf $(PWD)/$(DEP)/bench
	git clone $(BENCH_URL) $(PWD)/$(DEP)/bench
	mkdir -p $(PWD)/$(DEP)/bench/build
	cd $(PWD)/$(DEP)/bench/build && cmake -DBUILD_SHARED_LIBS=ON -DBENCHMARK_ENABLE_TESTING=OFF .. && cmake --build .


gtest: $(GLIB)

$(GLIB):
	rm -rf $(PWD)/$(DEP)/gtest
	git clone -b $(VERSION) https://github.com/google/googletest.git $(PWD)/$(DEP)/gtest
	mkdir -p $(PWD)/$(DEP)/gtest/build
	cd  $(PWD)/$(DEP)/gtest/build && \
	    $(CMAKE_GTEST_BLD) .. -Dgtest_disable_pthreads=ON && \
		cmake --build .


# boost: $(BOOST_DEP)

# $(BOOST_DEP):
# 	git clone --recursive https://github.com/boostorg/boost.git $(PWD)/$(DEP)/boost
# 	mkdir $(PWD)/$(DEP)/boost/build
# 	cd $(PWD)/$(DEP)/boost/build && cmake -DBUILD_SHARED_LIBS=ON .. && cmake --build .


hyperscan: $(HYPERSCAN_DEP)

$(HYPERSCAN_DEP):
	git clone https://github.com/intel/hyperscan.git $(PWD)/$(DEP)/hyperscan
	mkdir -p $(PWD)/$(DEP)/hyperscan/build
	cd $(PWD)/$(DEP)/hyperscan/build && cmake -DBUILD_SHARED_LIBS=ON -DBOOST_ROOT=../../boost .. && cmake --build .

$(TARGET) : hyperscan json yaml-cpp re2 build $(GLIB) $(LIBTARGET) $(TESTTARGET) $(BENCHTARGET) $(OBJ)
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

