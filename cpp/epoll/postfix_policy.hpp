#ifndef __POSTFIX_POLICY_HPP
#define __POSTFIX_POLICY_HPP

#include <string>

class PostfixPolicy {
private:
  int fd_;
  std::string msg_;

public:
  PostfixPolicy( int fd, const std::string & msg ): fd_(fd), msg_(msg){ };
  std::string read_until( const std::string & sep );
  std::string read_request();
};

#endif
