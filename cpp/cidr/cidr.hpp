// CIDR library for IP testing
#ifndef __CIDR_HPP
#define __CIDR_HPP

#include <string>
#include "ip.hpp"

class CIDR
{
private:
  IP::decimal_t _lower;
  IP::decimal_t _upper;

  public:
  CIDR( const std::string & cidr );
  bool in( const IP &);

  IP::decimal_t lower() const;
  IP::decimal_t upper() const;

};

#endif
