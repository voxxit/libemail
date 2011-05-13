#include "ip.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

IP::IP( decimal_t d ):_decimal( d ){ };

IP::IP( const std::string & ip )
  :_decimal(0)
{
  struct in_addr addr;
  inet_aton( ip.c_str() , &addr);
  _decimal = ntohl(addr.s_addr);
}

IP::decimal_t IP::decimal() const {
  return _decimal;
}

const char * IP::str() const{
  struct in_addr addr;
  addr.s_addr = htonl( _decimal );
  return inet_ntoa(addr);
}
