/*
 * A main function for the phtread tests against the
 * command CIDR datastructures
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "utility.hpp"

int main( int argc, char ** argv ){

  /* Call the initializer function, which will
   * populate our data structure for us, and we
   * just search the datastructure, looking for
   * each unsigned integer within the data struct
   */
  char * filename = NULL;
  if( argc < 2 ){
    //perror( "You must supply a filename as a CIDR whitelist input" );
    filename = "whitelist";
  }
  else {
    filename = argv[1];
  }

  cidr_pair_t * cp = NULL;
  readCIDRFromFile( filename, cp );

  printf("Done reading into cp\n");

  return 0;
}
