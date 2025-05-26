all: game

game: main.cpp
	g++ -o game main.cpp -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm -f game
