#include "cidr.hpp"
#include <iostream>
#include <algorithm>
#include <vector>

// A struct of the upper and lower ranges of each cidr object
// represented in its base types
typedef struct {
  IP::decimal_t l;
  IP::decimal_t u;
} cidr_pair_t;

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

    // Do the array stuff
    cidr_pair_t* cp = new cidr_pair_t[ whitelist.size() ];
    int j = 0;
    for( std::vector<CIDR>::iterator i  = whitelist.begin();
	 i != whitelist.end();
	 i++ ){
      cp[j].l = (*i).lower();
      cp[j].u = (*i).upper();
      j++;
    }

  }
  catch( std::string &e ){
    std::cerr << e << std::endl;
  }

  return 0;
}

// Generate a struct array of decimals, representing the upper
// and lower ends of the CIDR ranges
int createCidrArray( const std::vector< CIDR > & v ){
  

  return 0;
}
