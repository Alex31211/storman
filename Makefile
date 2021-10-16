all:
	gcc -g -O *.c -Wall -Wextra -Wno-unused-but-set-parameter -Wno-int-conversion -o program

clean:
	-rm program
