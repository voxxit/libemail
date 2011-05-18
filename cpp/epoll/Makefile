.PHONY=clean all

all:	epoll

clean:
	rm -f *.o epoll

epoll:	epoll.o
	g++ epoll.o -o epoll -lboost_thread

epoll.o:	epoll.cc
	g++ -c epoll.cc
