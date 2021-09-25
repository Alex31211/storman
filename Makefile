all:
	gcc -g -O *.c -Wall -Wextra -Wno-unused-but-set-parameter -o program

clean:
	-rm program
