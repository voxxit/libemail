// utility.cpp
//
// This file contains the functions accessed by both
// the C and C++ programmes, to read in the data,
// parse the data, and re-format the data structures
// into arrays of elemental types for distributed
// processing.
#include "cidr.hpp"
#include "utility.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

int readIPsFromStdIn( unsigned int * ipArray ){
  std::vector< IP > v;

  try {

    while( std::cin ){
      std::string strIP;
      std::cin >> strIP;

      if( strIP.size() == 0 )
	continue;

      v.push_back( IP( strIP ) );
    
    }
  }
  catch( std::string & e ){
    std::cerr << "Exception: " << e << std::endl;
    return -1;
  }

  if( v.size() > 0 ){
    int j = 0;
    ipArray = new unsigned int [ v.size() ];

    for( std::vector< IP >::iterator i = v.begin();
	 i != v.end();
	 i++ ){
      ipArray[j] = (*i).decimal();
    }
  }
  else
    return -1;

  return 0;
}

int readCIDRFromFile( const char * filename, cidr_pair_t * cp ){
  std::vector< CIDR > v;
  std::ifstream ifs ( filename, std::ifstream::in );
  char line[255];
  int count = 0;

  if( ! ifs.is_open() )
    return -1;

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
    cp = (cidr_pair_t *)malloc( sizeof(cidr_pair_t) * v.size() );
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

  return 0;
}
