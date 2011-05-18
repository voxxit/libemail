#ifndef __POSTFIX_POLICY_HPP
#define __POSTFIX_POLICY_HPP

#include <string>

class PostfixPolicy {
private:
  int fd_;

public:
  PostfixPolicy( int fd ): fd_(fd){ };
  std::string read_until( const std::string & sep );
  std::string read_request();
};

#endif
