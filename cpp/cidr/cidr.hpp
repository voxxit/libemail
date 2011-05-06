// CIDR library for IP testing
#ifndef __CIDR_HPP
#define __CIDR_HPP

#include <string>
#include "ip.hpp"

class CIDR
{
private:
  unsigned int _lower;
  unsigned int _upper;

  public:
  CIDR( const std::string & cidr );
  bool in( const IP &);
};

#endif
