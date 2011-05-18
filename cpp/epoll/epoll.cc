// Compile with: $ g++ epoll.cc -o epoll1 -lboost_thread
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <queue>

std::queue < int > socket_queue_;
std::queue < int > poll_queue_;
boost::interprocess::interprocess_condition cond_;
boost::mutex mutex_;
boost::mutex poll_mutex_;
boost::interprocess::interprocess_semaphore sem_(0);

class ConnectionHandler {
      public:
	void h() {
		int fd = 0;
		while (1) {
			sem_.wait();
			if (1) {
				boost::mutex::scoped_lock scoped_lock(mutex_);
				std::cout <<
				    "Getting FD from vector" << std::endl;
				if (socket_queue_.size() > 0) {
					fd = socket_queue_.front();
					socket_queue_.pop();
				} else
					fd = 0;
			}
			if (fd) {
				char buffer[255];
				int nread = read(fd, buffer, 255);
				if (nread < 1) {
					std::cerr << "Error reading socket" <<
					    std::endl;
					continue;
				}

				std::string msg = "Hi, from the internet\r\n";
				write(fd, msg.c_str(), msg.size());

				// Return
				if (1) {
					boost::mutex::scoped_lock scoped_lock(poll_mutex_);
					std::cout <<
					    "Returning FD to the epoll thing"
					    << std::endl;
					poll_queue_.push(fd);
				}
			}
		}
	};

};

int
create_sock()
{
	int
	    sockfd,
	    newsockfd,
	    portno;
	socklen_t clilen;
	char
	    buffer[256];
	struct sockaddr_in
	    serv_addr,
	    cli_addr;
	int
	    n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		perror("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(12345);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		perror("ERROR on binding");
	listen(sockfd, 5);

	return sockfd;
}

int
main()
{
#define MAX_EVENTS 1000
	struct epoll_event
	    ev,
	    events[MAX_EVENTS];
	int
	    listen_sock,
	    conn_sock,
	    nfds,
	    epollfd;
	struct sockaddr
	    serv_addr;
	struct sockaddr
	    local;
	socklen_t addrlen;

	/* Set up listening socket, 'listen_sock' (socket(),
	 * bind(), listen()) */

	listen_sock = create_sock();

	epollfd = epoll_create(MAX_EVENTS);
	if (epollfd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
	// Create a bunch of threads
	boost::thread_group tg;
	for (int j = 0; j < 100; j++) {
		tg.add_thread(new boost::thread(boost::bind(&ConnectionHandler::h, new ConnectionHandler())));
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, 100);
		if (nfds == -1) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}

		if (nfds == 0) {
			boost::mutex::scoped_lock scoped_lock(poll_mutex_);
			while (poll_queue_.size() > 0) {
				std::cout <<
				    "Retreiving FD from returning queue, and adding back to pollfd"
				    << std::endl;
				int returning_fd = poll_queue_.front();
				poll_queue_.pop();
				epoll_ctl(epollfd, EPOLL_CTL_ADD, returning_fd, &ev);
			}
		}

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				conn_sock = accept(listen_sock, (struct sockaddr *) &local, &addrlen);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				//setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD,
					conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				if (events[n].events & EPOLLIN == EPOLLIN) {
					std::cout << "EPOLLIN..." << std::endl;
					// Remove this socket from the 
					epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
					if (1) {
						boost::mutex::scoped_lock scoped_lock(mutex_);
						std::cout << "Adding fd to queue" << std::endl;
						socket_queue_.push(events[n].data.fd);
					}
					// Notify threads that there's a connection to process
					std::cout << "Posting semaphore" << std::endl;
					sem_.post();
				} else if (events[n].events & EPOLLHUP == EPOLLHUP) {
					std::cout << "EPOLHUP" << std::endl;
					std::cout << "Socket closed" << std::endl;
					epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, NULL);
					std::cout << "Done removing fd from pool" << std::endl;
				}
			}
		}
	}

	std::cout << "Joining all threads" << std::endl;
	tg.join_all();

}
