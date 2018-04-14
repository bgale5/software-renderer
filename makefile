engine: main.o engine.o viewer.o
	clang++ -std=c++14 -framework GLUT -framework OpenGL -framework Cocoa -o engine -g main.o engine.o viewer.o
main.o: main.cpp
	clang++ -c -g -std=c++14  -o main.o main.cpp
engine.o: engine.cpp engine.hpp
	clang++ -std=c++14 -c -g -std=c++14 -o engine.o engine.cpp
viewer.o: viewer.cpp viewer.hpp
	clang++ -std=c++14 -c -g -std=c++14 -o viewer.o viewer.cpp