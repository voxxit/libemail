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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <queue>
#include <map>

struct message {
  int fd;
  std::string msg;
};

std::queue < struct message > message_queue_;
std::queue < int > socket_queue_;
std::queue < int > poll_queue_;
boost::interprocess::interprocess_condition cond_;
boost::mutex mutex_;
boost::mutex poll_mutex_;
boost::interprocess::interprocess_semaphore sem_(0);

#include "postfix_policy.hpp"

int handler_number = 0;

class ConnectionHandler{
private:

  int fd_;
  int handler_num_;

  bool validate_socket(){
    char b;
    int valid = recv(fd_, &b, 1, MSG_PEEK | MSG_DONTWAIT);
    return valid > 0;
  }
public:
  ConnectionHandler():handler_num_(++handler_number){ };
  void h() {
    while (1) {
      message m;
      sem_.wait();
      if (1) {
	boost::mutex::scoped_lock scoped_lock(mutex_);
	std::cout << "[" << handler_num_ << "] Getting FD from vector" << std::endl;
	if (message_queue_.size() > 0) {
	  m = message_queue_.front();
	  fd_ = m.fd;
	} else
	  fd_ = 0;
      }
      if (fd_) {
	std::cout << "We would pass the string to the appropriate handler here" << std::endl;
	std::cout << "Message [" << m.msg << "]" << std::endl;
      }
    }
  };
};

static bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() > ending.length())
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    return false;
}

std::string read_from_socket( int fd, int * total_read, std::string until = "\r\n"  ){
  std::string buffer;
  char b[2];
  b[1] = '\0';
  int nread;
  *total_read = 0;
  while (1){
    nread = read( fd, b, 1 );
    *total_read = nread;
    if( nread > 0 ){
      std::cout << "Read [" << b << "]" << std::endl;
      buffer.append( b );
      if( hasEnding( buffer, until ) )
	return buffer;
    }
    else
      return buffer;
  }
  return buffer;
}

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

  int on = 1;
  setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

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
#define MAX_EVENTS 10000
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

  std::map< int, std::string > pending_data;

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
      // This is where we attempt to re-introduce a fd
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
	// Read whatever is in the socket, and if it's a complete line, then
	// pass it to a worker thread to do something with it.
	int nread;
	std::string some_input = read_from_socket( events[n].data.fd, &nread );
	if( nread == 0 ){
	  std::cout << "Socket closed: " << events[n].data.fd << std::endl;
	  epoll_ctl( epollfd, EPOLL_CTL_DEL, events[n].data.fd, &ev );
	  
	  boost::mutex::scoped_lock scoped_lock(mutex_);
	  std::map< int, std::string >::iterator i = pending_data.find( events[n].data.fd );
	  pending_data.erase( i );
	  close( events[n].data.fd );
	  continue;
	}

	std::cout << "Read: [" << some_input << "]" << std::endl;

	std::string & str_ref = pending_data[ events[n].data.fd ];
	str_ref.append( some_input );
	std::cout << "Data so far: [" << str_ref << "]" << std::endl;


	if( hasEnding( pending_data[ events[n].data.fd ] , "\r\n\r\n" ) ){
	  std::cout << "End of data!!" << std::endl;
	  boost::mutex::scoped_lock scoped_lock(mutex_);
	  struct message m;
	  m.fd = events[n].data.fd;
	  m.msg = std::string( pending_data[ events[n].data.fd ] );
	  message_queue_.push( m );

	  std::map< int, std::string >::iterator i = pending_data.find( events[n].data.fd );
	  pending_data.erase( i );
	 
	  std::cout << "Posting semaphore" << std::endl;
	  sem_.post();
	}
      }
    }
  }

  std::cout << "Joining all threads" << std::endl;
  tg.join_all();

}
