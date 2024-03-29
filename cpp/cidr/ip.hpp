#ifndef __IP_HPP
#define __IP_HPP
#include <string>

class IP {
public:
  typedef unsigned long decimal_t;
private:
  decimal_t _decimal;

public:
  IP( const std::string & ip );
  IP( decimal_t d );
  decimal_t decimal() const;

  bool operator <  (const IP & rhs) const { return ( _decimal  < rhs._decimal);};

  const char * str() const;

};

#endif
