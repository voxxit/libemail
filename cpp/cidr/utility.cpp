// utility.cpp
//
// This file contains the functions accessed by both
// the C and C++ programmes, to read in the data,
// parse the data, and re-format the data structures
// into arrays of elemental types for distributed
// processing.
#include "utility.hpp"
#include "cidr.hpp"
#include "utility.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory.h>

unsigned long * readIPsFromStdIn( int * nips ){
  std::vector< IP > v;

  *nips = 0;

  try {

    while( std::cin ){
      std::string strIP;
      std::cin >> strIP;

      if( strIP.size() == 0 )
	continue;

      IP i( strIP );
      std::cout << "Adding IP " << strIP << "\t" << i.decimal() << std::endl;
      v.push_back( i );
    
    }
  }
  catch( std::string & e ){
    std::cerr << "Exception: " << e << std::endl;
    return NULL;
  }

  unsigned long *ipArray;
  if( v.size() > 0 ){
    int j = 0;
    ipArray = (unsigned long *)malloc( sizeof(unsigned long)*v.size() + 1 );
    std::cout << "Created array at " << ipArray << std::endl;
    memset( ipArray, sizeof( unsigned long ) * v.size() + 1, 0 );

    for( std::vector< IP >::iterator i = v.begin();
	 i != v.end();
	 i++ ){
      ipArray[j] = (unsigned long)((*i).decimal());
      std::cout << "Copied " << ipArray[j] << " to array[" << j << "] " << &ipArray[j] << std::endl;
      j++;
    }
  }
  else
    return NULL;

  *nips = v.size();
  std::cout << "Returning pointer " << ipArray << std::endl;
  return ipArray;
}

cidr_pair_t* readCIDRFromFile( const char * filename ){
  std::vector< CIDR > v;
  std::ifstream ifs ( filename, std::ifstream::in );
  char line[255];
  int count = 0;
  cidr_pair_t * cp;

  if( ! ifs.is_open() )
    return NULL;

  while (ifs.good()){
    ifs.getline( line, 254 );
    std::string s(line);

    if( s.size() < 1 ){
      std::cerr << "Size is less than 1" << std::endl;
      break;
    }

    std::cout << "Read: [" << s << "]" << std::endl;

    if(s.size() > 0){
      CIDR c( s );
  
      v.push_back( c );
	    
      std::cout << "Done with this CIDR" << std::endl;
    }
  }
  
  if( v.size() > 0 ){
    // Ugh, use malloc, as it may have to be free()'d by a C program
    cp = (cidr_pair_t *)malloc( sizeof(cidr_pair_t) * v.size() + 1 );
    printf("CIDR Pair address: %p\n", (void *)cp );
    memset( cp, sizeof( cidr_pair_t ) * v.size() + 1, 0 );

    int j = 0;
    for( std::vector<CIDR>::iterator i = v.begin();
         i != v.end();
         i++ ){
      cp[j].l = (*i).lower();
      cp[j].u = (*i).upper();
      j++;
    }
  } 

  std::cout << "Done. Returning" << std::endl;
  ifs.close();

  return cp;
}
