# PAPERLAND build (Linux / MSYS2 / MinGW).  Usage:  make   |  make run  |  make clean
# Every .cpp under src/ (core, shared, chapters/*) is compiled automatically,
# so adding a chapter folder needs no Makefile change.
CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Iinclude
SRC      := $(shell find src -name '*.cpp')
HDR      := $(shell find include -name '*.h')
ifeq ($(OS),Windows_NT)
  LIBS := -lfreeglut -lopengl32 -lglu32
  OUT  := PAPERLAND.exe
else
  LIBS := -lglut -lGLU -lGL
  OUT  := PAPERLAND
endif

$(OUT): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
