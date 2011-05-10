#ifndef __UTILITY_HPP
#define __UTILITY_HPP



  // Struct for containing native datatype of CIDR
  typedef struct {
    unsigned int u;
    unsigned int l;
  } cidr_pair_t;

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

  int readIPsFromStdIn( unsigned int * ipArray );
  int readCIDRFromFile( const char * filename, cidr_pair_t * cp );

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
