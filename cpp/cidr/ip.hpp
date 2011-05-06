#ifndef __IP_HPP
#define __IP_HPP
#include <string>

class IP {
private:
  unsigned int _decimal;

public:
  IP( const std::string & ip );
  unsigned int decimal() const;

};

#endif
