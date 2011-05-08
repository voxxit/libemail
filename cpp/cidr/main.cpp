#include "cidr.hpp"
#include <iostream>
#include <algorithm>
#include <vector>

int main(void){
  try{

    std::vector< CIDR > whitelist;

    whitelist.push_back( CIDR ( "192.168.3.0/24" ) );
    whitelist.push_back( CIDR ( "192.168.6.0/24" ) );
    whitelist.push_back( CIDR ( "192.168.9.0/24" ) );
    whitelist.push_back( CIDR ( "192.168.22.0/24" ) );

    while( std::cin ){
  
      std::string strIP;			  
      std::cin >> strIP;

      if( strIP.size() == 0 )
	continue;

      std::cout << "Processing " << strIP << std::endl;

      // Create an iterator
      std::vector< CIDR >::iterator i = whitelist.begin();

      for( ; i != whitelist.end(); i++ ){
	if( (*i).in( IP( strIP ) ) ){
	  std::cout << "IP " << strIP << " is whitelisted" << std::endl;
	  break;
	}
      }
    }
  }
  catch( std::string &e ){
    std::cerr << e << std::endl;
  }

  return 0;
}
