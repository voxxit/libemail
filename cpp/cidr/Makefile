.PHONY=all clean test

all:	t t.pthread

test:	t
	./t < ip.list

clean:
	rm -f *.o t t.pthread

t:	main.o cidr.o ip.o utility.o
	g++ main.o cidr.o ip.o utility.o -o t

ip.o:	ip.cpp
	g++ -g -c ip.cpp

main.o:	main.cpp
	g++ -g -c main.cpp

cidr.o:	cidr.cpp
	g++ -g -c cidr.cpp

utility.o:	utility.cpp
	g++ -g -c utility.cpp

main_pthread.o:	main_pthread.c
	gcc -g -c main_pthread.c

t.pthread:	main_pthread.o utility.o ip.o cidr.o
	gcc main_pthread.o utility.o ip.o cidr.o -lpthread -lstdc++ -o t.pthread
