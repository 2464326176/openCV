src = $(wildcard *.c)
cpp_src = $(wildcard *.cpp)
targets = $(patsubst %.c, %, $(src))
cpp_targets = $(patsubst %.cpp, %, $(cpp_src))

CXX = g++
CC = gcc
CFLAGS = -Wall -g -lpthread $(shell pkg-config --cflags opencv4) $(shell pkg-config --libs --static opencv4)


all:$(targets) $(cpp_targets)
	@echo $(targets)

$(targets):%:%.c
	$(CC) $< -o $@ $(CFLAGS)

$(cpp_targets):%:%.cpp
	$(CXX) $< -o $@ $(CFLAGS)

.PHONY:clean all
clean:
	-rm -rf $(targets) $(cpp_targets) a.out


#CXX ?= g++
#
#CXXFLAGS += -c -Wall $(shell pkg-config --cflags opencv)
#LDFLAGS += $(shell pkg-config --libs --static opencv)
#
#all: opencv_example
#
#opencv_example: example.o; $(CXX) $< -o $@ $(LDFLAGS)
#
#%.o: %.cpp; $(CXX) $< -o $@ $(CXXFLAGS)
#
#clean: ; rm -f example.o opencv_example


