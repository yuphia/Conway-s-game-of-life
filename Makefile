all: compile

compile: main.c
	gcc -Werror -Wpedantic -Wextra -Wall -o game.o main.c -lncurses

run: 
	./game.o
