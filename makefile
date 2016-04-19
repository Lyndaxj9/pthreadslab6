all: lab6.c
	gcc -o lab6 lab6.c -lpthread

clean:
	-rm lab6
