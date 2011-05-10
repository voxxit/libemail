#ifndef __UTILITY_HPP
#define __UTILITY_HPP



  // Struct for containing native datatype of CIDR
  typedef struct {
    unsigned long u;
    unsigned long l;
  } cidr_pair_t;

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

  unsigned long * readIPsFromStdIn(int*);
  cidr_pair_t * readCIDRFromFile( const char * filename );

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
