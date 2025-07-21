CFLAGS=-g3 -W -Wall
OBJECTS=command.o string_parser.o main.o
#all: simple-shell

simple-shell: string_parser.o command.o main.o
	gcc -o $@ $^

main.o: main.c command.h
	gcc -c -g3 main.c

command.o: command.c command.h
	gcc -c -g3 command.c

string_parser.o: string_parser.c string_parser.h
	gcc -c -g3 string_parser.c

clean:
	rm -f core *.o simple-shell
