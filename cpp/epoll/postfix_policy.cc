#include "postfix_policy.hpp"
#include <string>
#include <iostream>

static bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


std::string PostfixPolicy::read_until( const std::string & sep ){
  char buf;
  std::string buffer;

  while( 1 ){
    int nread = read( fd_, &buf, 1 );
    if( nread > 0 ){
      buffer.push_back( buf );
      if( hasEnding( buffer, sep ) ){
	return buffer.erase( buffer.size() - sep.size(), sep.size() );
      }
    }
    else
      return buffer;
  }
  return buffer;
}

std::string PostfixPolicy::read_request(){
  std::string request = read_until( "\r\n\r\n" );
  return request;
}
