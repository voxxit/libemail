.PHONY=all clean

all:	t

clean:
	rm *.o t

t:	main.o cidr.o ip.o
	g++ main.o cidr.o ip.o -o t

ip.o:	ip.cpp
	g++ -g -c ip.cpp

main.o:	main.cpp
	g++ -g -c main.cpp

cidr.o:	cidr.cpp
	g++ -c -c cidr.cpp
