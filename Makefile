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

BOOST_VERSION := 1_58_0

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

requires/boost-$(OS).flag: requires/boost-$(OS)
	cd $< ; ./bootstrap.sh $(BOOST_BOOTSTRAP_FLAGS)
	sed -i "s/mingw/gcc/g" $</project-config.jam
	cd $< ; ./b2 $(BOOST_B2_FLAGS)
	touch $@

requires-boost: requires/boost-$(OS).flag


STENCILA_VERSION := 0.18

requires/stencila-$(STENCILA_VERSION).zip:
	@mkdir -p requires
	wget --no-check-certificate -O $@ https://github.com/stencila/stencila/archive/$(STENCILA_VERSION).zip

requires/stencila: requires/stencila-$(STENCILA_VERSION).zip
	rm -rf requires/stencila
	unzip $< -d requires
	mv requires/stencila-$(STENCILA_VERSION) requires/stencila
	touch $@

requires-stencila: requires/stencila


FSL_VERSION := 1d2eb9c7a35b498cd30d990804365e32aef0d8c1

requires/fsl-$(FSL_VERSION).zip:
	@mkdir -p requires
	wget --no-check-certificate -O $@ https://github.com/trident-systems/fsl/archive/$(FSL_VERSION).zip

requires/fsl: requires/fsl-$(FSL_VERSION).zip
	rm -rf requires/fsl
	unzip $< -d requires
	mv requires/fsl-$(FSL_VERSION) requires/fsl
	touch $@

requires-fsl: requires/fsl


requires: requires-boost requires-stencila requires-fsl


#############################################################
# Executables
 
# Define compile options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs -pthread
INC_DIRS := -I. -Irequires/boost-$(OS) -Irequires/stencila -Irequires/fsl
LIB_DIRS := -Lrequires/boost-$(OS)/lib
LIBS := 

# Find all .hpp and .cpp files (to save time don't recurse into subdirectories)
HPPS := $(shell find . -maxdepth 1 -name "*.hpp")
CPPS := $(shell find . -maxdepth 1 -name "*.cpp")

# Executable for normal use
sna1.exe: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)

# Executable for debugging
sna1.debug: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -g -O0 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)

# Executable for profiling
sna1.prof: $(HPPS) $(CPPS)
	$(CXX) $(CXX_FLAGS) -pg -O3 $(INC_DIRS) -o$@ sna1.cpp $(LIB_DIRS) $(LIBS)


#############################################################
# Running
 
run: sna1.exe
	time ./sna1.exe
