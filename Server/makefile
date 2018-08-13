# makefile for teststack
# the filename must be either Makefile or makefile

myftpd: myftpd.o stream.o
	gcc myftpd.o stream.o -o myftpd

myftpd.o: myftpd.c
	gcc -c myftpd.c

stream.o: stream.c stream.h
	gcc -c stream.c

clean: 
	rm *.o
