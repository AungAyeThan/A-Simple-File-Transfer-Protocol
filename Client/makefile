# makefile for teststack
# the filename must be either Makefile or makefile

myftp: myftp.o stream.o
	gcc myftp.o stream.o -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

stream.o: stream.c stream.h
	gcc -c stream.c

clean: 
	rm *.o
