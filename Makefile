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

BOOST_VERSION := 1_62_0

requires/boost_$(BOOST_VERSION).tar.bz2:
	@mkdir -p requires
	wget --no-check-certificate -O $@ http://prdownloads.sourceforge.net/boost/boost_$(BOOST_VERSION).tar.bz2

requires/boost: requires/boost_$(BOOST_VERSION).tar.bz2
	tar --bzip2 -xf $< -C requires
	mv requires/boost_$(BOOST_VERSION) requires/boost
	touch $@

BOOST_BOOTSTRAP_FLAGS := --with-libraries=filesystem,test
BOOST_B2_FLAGS := -d0 --prefix=. link=static install
ifeq ($(OS), linux)
	BOOST_B2_FLAGS += cxxflags=-fPIC
endif
ifeq ($(OS), win)
	BOOST_BOOTSTRAP_FLAGS += --with-toolset=mingw
	BOOST_B2_FLAGS += --layout=system release toolset=gcc
endif

requires/boost/lib: requires/boost
	cd $< ; ./bootstrap.sh $(BOOST_BOOTSTRAP_FLAGS)
	sed -i "s/mingw/gcc/g" $</project-config.jam
	cd $< ; ./b2 $(BOOST_B2_FLAGS)
	touch $@


STENCILA_VERSION := 0.2

requires/stencila-cpp-$(STENCILA_VERSION).zip:
	@mkdir -p requires
	wget --no-check-certificate -O $@ https://github.com/stencila/cpp/archive/$(STENCILA_VERSION).zip

requires/stencila: requires/stencila-cpp-$(STENCILA_VERSION).zip
	rm -rf requires/stencila
	unzip -o $< -d requires
	mv requires/cpp-$(STENCILA_VERSION)/ requires/stencila/
	touch $@


requires/r-packages.installed:
	Rscript -e "install.packages(c('tidyr','dplyr','ggplot2'), repos='http://cran.us.r-project.org')"
	touch $@


requires: requires/boost/lib requires/stencila requires/r-packages.installed


#############################################################
# Executables
 
# Define compile options and required libraries
CXX_FLAGS := -std=c++11 -Wall -Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-variable -pthread
INC_DIRS := -I. -Irequires/boost -Irequires/stencila
LIB_DIRS := -Lrequires/boost/lib
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
# The `no-inline` options provide better coverage statistics because lines don't get "inlined away"
tests-fast.exe: $(HPPS) tests/fast.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -g -O0 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline $(INC_DIRS) -o$@ tests/fast.cpp $(LIB_DIRS) $(LIBS) -lboost_unit_test_framework -lgcov

# Test executable with no coverage and full optimisation for tests that run very slowly without these
tests-slow.exe: $(HPPS) tests/slow.cpp $(TEST_CPPS) requires
	$(CXX) $(CXX_FLAGS) -O3 $(INC_DIRS) -o$@ tests/slow.cpp $(LIB_DIRS) $(LIBS) -lboost_unit_test_framework


#############################################################
# Running
 
# Run the model
run: sna1.exe
	time ./sna1.exe run


#############################################################
# Testing

# CASAL v230 for a linux binary
tests/casal/casal-230.zip:
	wget -O $@ ftp://ftp.niwa.co.nz/software/casal/CASALv230-2012-03-21.zip

# CASAL "latest" for a version of the R package that is compatible
# with recent R versions
tests/casal/casal-latest.zip:
	wget -O $@ ftp://ftp.niwa.co.nz/software/casal/latest_casal.zip

# Unzip a CASAL zip
tests/casal/casal-%: tests/casal/casal-%.zip
	rm -rf $@
	mkdir $@
	unzip $< -d $@

# Install CASAL
#   - R package
#   - symbolic link to Linux executable
casal/casal.installed: casal/casal-230 casal/casal-latest
	R CMD INSTALL casal/casal-latest/R_library/casal_2.30.tar.gz
	chmod 755 casal/casal-230/Program/Linux/casal
	ln -sf casal-230/Program/Linux/casal tests/casal/casal
	touch $@

# Run fast tests
tests-fast: tests-fast.exe
	time ./tests-fast.exe

# Run slow tests
tests-slow: tests-slow.exe casal/casal.installed
	time ./tests-slow.exe

# Run all tests
test: tests-fast tests-slow
