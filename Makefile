CXXFLAGS=-std=c++17 -rdynamic -O3 -funroll-loops -fdiagnostics-color=always -Wall -Wextra -Wno-error=unused -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-unused-parameter -Wnull-dereference -Werror -Wno-deprecated-declarations
ifeq ($(OS), Windows_NT)
    UNAME := Windows
else
    UNAME := $(shell uname)
endif

ifeq (${UNAME}, Linux)
    LIBS = -lrt
else
    LIBS =
endif

RCLIB=$(wildcard RC/*.h)
TARGET=runvenv

all: $(TARGET) helpers

$(TARGET): runvenv.cpp $(RCLIB)
	$(CXX) -o runvenv runvenv.cpp $(CXXFLAGS) $(LIBS)

ifeq (${UNAME}, Windows)
runpip: $(TARGET)
	cp runvenv runpip
else
runpip: $(TARGET)
	ln -sf runvenv runpip
endif

ifeq (${UNAME}, Windows)
runipyth: $(TARGET)
	cp runvenv runipyth
else
runipyth: $(TARGET)
	ln -sf runvenv runipyth
endif

helpers: runpip runipyth

clean:
	rm -f $(TARGET)

