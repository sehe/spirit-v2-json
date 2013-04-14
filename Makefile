all:test

CPPFLAGS+=-std=c++0x
CPPFLAGS+=-g -O0
CPPFLAGS+=-I ~/custom/boost/

# CPPFLAGS+=-fopenmp
# CPPFLAGS+=-march=native

# LDFLAGS+=-L ~/custom/boost/stage/lib/
# LDFLAGS+=-lboost_system -lboost_regex -lboost_thread -lpthread

CXX=/usr/lib/gcc-snapshot/bin/g++
CC=/usr/lib/gcc-snapshot/bin/gcc

CXX=clang++
CC=clang

test: | JSON.hpp

%:%.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
