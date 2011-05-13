#include "cidr.hpp"
#include "utility.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

// Forward decl
static int createCidrArray( const std::vector< CIDR > & v, cidr_pair_t * cp );

void print_whitelisted( IP::decimal_t & whitelisted ){
  IP i(whitelisted);
  std::cout << "Whitelisted: " << i.str() << std::endl;
}

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

  }
  catch( std::string &e ){
    std::cerr << e << std::endl;
  }

  std::vector< IP::decimal_t > v;
  try {
    while( std::cin ){
      std::string strIP;
      std::cin >> strIP;

      if( strIP.size() == 0 )
	continue;

      IP i( strIP );
      v.push_back( i.decimal() );
    }
    std::sort( v.begin(), v.end() );
  }
  catch( std::string & e ){
    std::cerr << "Exception: " << e << std::endl;
    return NULL;
  }

  std::cout << "Created array for subsequent work.."
            << std::endl;

  // We have a sorted array of unsigned long.  Go through
  // each whitelist, and find all IPs within its bounds

  for( std::vector< CIDR >::iterator i = whitelist.begin();
       i != whitelist.end();
       i++ ){
    IP::decimal_t lower = (*i).lower();
    IP::decimal_t upper = (*i).upper();

    // Find 
    std::vector< IP::decimal_t >::iterator lb = std::lower_bound( v.begin(), v.end(), lower );
    std::vector< IP::decimal_t >::iterator ub = std::upper_bound( v.begin(), v.end(), upper );

    std::for_each( lb, ub, print_whitelisted );
  }

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
