# PAPERLAND build (Linux / MSYS2 / MinGW).  Usage:  make   |  make run  |  make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall
SRC      := $(wildcard src/*.cpp)
ifeq ($(OS),Windows_NT)
  LIBS := -lfreeglut -lopengl32 -lglu32
  OUT  := PAPERLAND.exe
else
  LIBS := -lglut -lGLU -lGL
  OUT  := PAPERLAND
endif

$(OUT): $(SRC) $(wildcard include/*.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
