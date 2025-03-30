CXXFLAGS=-std=c++17 -rdynamic -O3 -funroll-loops -fdiagnostics-color=always -Wall -Wextra -Wno-error=unused -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-unused-parameter -Wnull-dereference -Werror
LIBS=-lrt
RCLIB=$(wildcard RC/*.h)
TARGET=autovenv

$(TARGET): autovenv.cpp $(RCLIB)
	$(CXX) -o autovenv autovenv.cpp $(CXXFLAGS) $(LIBS)

all: $(TARGET)

clean:
	rm -f $(TARGET)

