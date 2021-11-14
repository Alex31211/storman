all:
	gcc -ggdb *.c -Wall -Wextra -O0 -fsanitize=undefined -o program

clean:
	-rm program
