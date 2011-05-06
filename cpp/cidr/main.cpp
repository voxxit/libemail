#include "cidr.hpp"
#include <iostream>

int main(void){
  try{
    CIDR c( "192.168.3.0/24" );
   
    if( c.in( IP("192.168.3.5") ) ){
      std::cout << "IP is in CIDR" << std::endl;
    }
    else {
      std::cerr << "IP not found in CIDR" << std::endl;
    }
  }
  catch( std::string &e ){
    std::cerr << e << std::endl;
  }

  return 0;
}
