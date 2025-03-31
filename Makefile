CXXFLAGS=-std=c++17 -rdynamic -O3 -funroll-loops -fdiagnostics-color=always -Wall -Wextra -Wno-error=unused -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-unused-parameter -Wnull-dereference -Werror -Wno-deprecated-declarations
LIBS=-lrt
RCLIB=$(wildcard RC/*.h)
TARGET=runvenv

$(TARGET): runvenv.cpp $(RCLIB)
	$(CXX) -o runvenv runvenv.cpp $(CXXFLAGS) $(LIBS)

all: $(TARGET)

clean:
	rm -f $(TARGET)

