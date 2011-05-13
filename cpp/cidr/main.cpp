#include "cidr.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

// A struct of the upper and lower ranges of each cidr object
// represented in its base types
typedef struct {
  IP::decimal_t l;
  IP::decimal_t u;
} cidr_pair_t;

// Forward decl
static int createCidrArray( const std::vector< CIDR > & v, cidr_pair_t * cp );

int main(int argc, char **argv){

  std::vector< CIDR > whitelist;

  try{
    std::ifstream ifs ( argv[1], std::ifstream::in );

    while( ifs.good() ){
       char l[255];
       ifs.getline( l, 254 );
       std::string sline( l );

       if( sline.size() < 1 )
         continue;
       
       whitelist.push_back( CIDR( sline ) );
    }
    ifs.close();

    while( std::cin ){
  
      std::string strIP;			  
      std::cin >> strIP;

      if( strIP.size() == 0 )
	continue;

      //std::cout << "Processing " << strIP << std::endl;

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

  // Create an array from our fancy OO objects :)
  cidr_pair_t *cp;

  if( createCidrArray( whitelist, cp ) != 0 ){
    // Problems
    std::cerr << "ERR: problem with createCidrArray fn"
              << std::endl;
    exit(1);
  }

  std::cout << "Created array for subsequent work.."
            << std::endl;

  return 0;
}

// Generate a struct array of decimals, representing the upper
// and lower ends of the CIDR ranges
//
// Returns  0 on success,
// Returns -1 on failure to allocate cidr_pair_t pointer
int createCidrArray( const std::vector< CIDR > & v, cidr_pair_t * cp ){
  if( v.size() < 1 )
    return -1;

  cp = new cidr_pair_t [ v.size() ];

  if( cp == NULL )
    return -1;

  // Have a secondary index integer for traversing
  // the cp array
  int j = 0;

  // Iterate through "v", and populate the cp array
  for( std::vector< CIDR >::const_iterator i = v.begin();
       i != v.end();
       i++ )
    {
      cp[j].l = (*i).lower();
      cp[j].u = (*i).upper();
      j++;
    }

  return 0;
}
