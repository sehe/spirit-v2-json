all:test

CPPFLAGS+=-std=c++0x
CPPFLAGS+=-g -O0 -Wall
CPPFLAGS+=-isystem ~/custom/boost/

# CPPFLAGS+=-fopenmp
# CPPFLAGS+=-march=native

LDFLAGS+=-L ~/custom/boost/stage/lib/
LDFLAGS+=-lboost_system -lboost_regex -lboost_thread -lpthread

CXX=clang++
CC=clang

CXX=/usr/lib/gcc-snapshot/bin/g++
CC=/usr/lib/gcc-snapshot/bin/gcc

test.o JSON.o: JSON.hpp

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $< -c -o $@

test:test.o JSON.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
