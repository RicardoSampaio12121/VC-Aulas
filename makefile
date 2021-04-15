output: p1.o vc.o
		gcc p1.o vc.o -o p1

p1.o: p1.c
		gcc -c p1.c

vc.o: vc.c vc.h
	  gcc -c vc.c

clean: 
	   rm +*.o prog