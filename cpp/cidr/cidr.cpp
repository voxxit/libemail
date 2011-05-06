#include "cidr.hpp"
#include <iostream>
// For address conversion stuff
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// atoi
#include <stdlib.h>
// pow
#include <math.h>

#include "ip.hpp"

CIDR::CIDR( const std::string & cidr ){
  struct in_addr addr;
  // Split the CIDR into the dotted quad, and the
  // power

  std::string ip,
    pow;

  std::string::size_type pos = cidr.find_first_of( "/", 0 );
  if( pos != std::string::npos ){
    ip = cidr.substr( 0, pos );
    pow = cidr.substr( pos + 1, cidr.length() );

    // std::cout << "ip  : " << ip << std::endl;
    // std::cout << "pow : " << pow << std::endl;
  }
  else {
    throw("Could not parse: " + cidr );
  }
  
  IP i( ip );
  _lower = i.decimal();
  int p = atoi(pow.c_str());
  _upper = _lower + ( ::pow( 2, 32-p)-1);

  // std::cout << "Lower: " << _lower << std::endl;
  // std::cout << "Upper: " << _upper << std::endl;
}

bool CIDR::in( const IP & ip ){
  IP i( ip );
  unsigned int id = i.decimal();
  if( ( id <= _upper ) )
    if( ( id >= _lower ))
      return true;
  return false;
}
