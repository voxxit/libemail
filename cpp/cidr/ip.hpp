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
  decimal_t decimal() const;


};

#endif
