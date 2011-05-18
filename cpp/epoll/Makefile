.PHONY=clean all

all:	epoll

clean:
	rm -f *.o epoll

epoll:	epoll.o postfix_policy.o
	g++ epoll.o postfix_policy.o -o epoll -lboost_thread

epoll.o:	epoll.cc
	g++ -c epoll.cc

postfix_policy.o:	postfix_policy.cc
	g++ -c postfix_policy.cc
