CXX := g++
CXXFLAGS := -o

BIN := temp/
LIBS := lib/
OUT := deploy/
LINKS := -lopenGL32 -lmingw32 -lSDL2main -lSDL2 -lglew32
OBJECTS := $(BIN)camera.o $(BIN)window.o $(BIN)shader.o $(BIN)curve.o
MAIN := $(CXX) $(CXXFLAGS) $(OUT)curves.exe $(OBJECTS) main.cpp $(LINKS)

main: main.cpp $(OBJECTS)
	$(MAIN)

$(BIN)camera.o: $(LIBS)camera.cpp $(LIBS)camera.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)camera.o $(LIBS)camera.cpp

$(BIN)window.o: $(LIBS)window/window.cpp $(LIBS)window/window.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)window.o $(LIBS)window/window.cpp

$(BIN)shader.o: $(LIBS)shader.cpp $(LIBS)shader.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)shader.o $(LIBS)shader.cpp

$(BIN)curve.o: $(LIBS)curve.cpp $(LIBS)curve.hpp
	$(CXX) -c $(CXXFLAGS) $(BIN)curve.o $(LIBS)curve.cpp

prepare:
	mkdir $(BIN) $(OUT)

clean:
	cd temp & del /q /s "*.o" & cd .. & $(MAKE) --no-print-directory main