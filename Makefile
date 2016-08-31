all: sna1.exe

clean:
	rm -f *.debug *.exe *.o

#############################################################
# Operating system

UNAME := $(shell uname -o)
ifeq ($(UNAME), GNU/Linux)
	OS := linux
endif
ifeq ($(UNAME), Msys)
	OS := win
endif

#############################################################
# Requirements

BOOST_VERSION := 1_61_0

requires/boost_$(BOOST_VERSION).tar.bz2:
	@mkdir -p requires
	wget --no-check-certificate -O $@ http://prdownloads.sourceforge.net/boost/boost_$(BOOST_VERSION).tar.bz2

requires/boost-$(OS): requires/boost_$(BOOST_VERSION).tar.bz2
	tar --bzip2 -xf $< -C requires
	mv requires/boost_$(BOOST_VERSION) requires/boost-$(OS)
	touch $@

BOOST_BOOTSTRAP_FLAGS := --with-libraries=filesystem,regex,test
BOOST_B2_FLAGS := -d0 --prefix=. link=static install
ifeq ($(OS), linux)
	BOOST_B2_FLAGS += cxxflags=-fPIC
endif
ifeq ($(OS), win)
	BOOST_BOOTSTRAP_FLAGS += --with-toolset=mingw
	BOOST_B2_FLAGS += --layout=system release toolset=gcc
endif

requires/boost-$(OS)/lib: requires/boost-$(OS)
	cd $< ; ./bootstrap.sh $(BOOST_BOOTSTRAP_FLAGS)
	sed -i "s/mingw/gcc/g" $</project-config.jam
	cd $< ; ./b2 $(BOOST_B2_FLAGS)
	touch $@


REFLECT_VERSION := 0.2

requires/reflect-$(REFLECT_VERSION).zip:
	@mkdir -p requires
	wget --no-check-certificate -O $@ https://github.com/stencila/reflect/archive/$(REFLECT_VERSION).zip

requires/reflect: requires/reflect-$(REFLECT_VERSION).zip
	rm -rf requires/reflect
	unzip $< -d requires
	mv requires/reflect-$(REFLECT_VERSION) requires/reflect
	touch $@


requires: requires/boost-$(OS)/lib requires/reflect


#############################################################
# Executables
 
# Define compile options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-variable -pthread
INC_DIRS := -I. -Irequires/boost-$(OS) -Irequires/reflect
LIB_DIRS := -Lrequires/boost-$(OS)/lib
LIBS := -lboost_system -lboost_filesystem 

# Find all .hpp and .cpp files (to save time don't recurse into subdirectories)
HPPS := $(shell find . -maxdepth 1 -name "*.hpp")
CPPS := $(shell find . -maxdepth 1 -name "*.cpp")
TEST_CPPS = $(wildcard tests/*.cpp)

# Executable for normal use
sna1.exe: $(HPPS) $(CPPS) requires
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)

# Executable for debugging
sna1.debug: $(HPPS) $(CPPS) requires
	$(CXX) $(CXX_FLAGS) -g -O0 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)

# Executable for profiling
sna1.prof: $(HPPS) $(CPPS) requires
	$(CXX) $(CXX_FLAGS) -pg -O3 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)

# Test executable with coverage (and no optimisation) for tests that run fast
# These tests are likely to be run often during development and don't require optimisation
tests-fast.exe: $(HPPS) tests/tests-fast.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -g -O0 --coverage $(INC_DIRS) -o$@ tests/tests-fast.cpp $(LIB_DIRS) $(LIBS) -lboost_unit_test_framework -lgcov

# Test executable with no coverage and full optimisation for tests that run very slowly without these
tests-slow.exe: $(HPPS) tests/tests-slow.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ tests/tests-slow.cpp $(LIB_DIRS) $(LIBS) -lboost_unit_test_framework


#############################################################
# Running
 
run: sna1.exe
	time ./sna1.exe run


#############################################################
# Testing

tests-fast: tests-fast.exe
	time ./tests-fast.exe

tests-slow: tests-slow.exe
	time ./tests-slow.exe

test: tests-fast tests-slow
