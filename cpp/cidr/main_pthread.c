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

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef struct {
  cidr_pair_t * whitelist;
  unsigned long * start;
  unsigned long * end;
  int id;
} work_data_t;

void * thread_start( void * arg ){
  work_data_t * work = (work_data_t *)arg;
  unsigned long * ip = work->start;
  
  printf("%d\tFirst unit of work at %p\t", work->id, (void *)ip );
  printf("%d\tLast  unit of work at %p\n", work->id, (void *)work->end );

  while( ip && *ip ){
    /* Check if this guy is in the CIDR whitelists */
    /* For now, just check against the first CIDR range... */
#ifdef DEBUG
    printf("%d\t%p\n", work->id, ip);
#endif

    /* iterate through the CIDR whitelist */
    cidr_pair_t *c;
    struct in_addr in_l, in_u;

    struct in_addr ip_addr;
    ip_addr.s_addr = htonl(*ip);

    //printf( "%d\tLooking up ip: %s => %u\n", work->id, inet_ntoa( ip_addr ), *ip);

    c = work->whitelist;

    while( (*c).u && (*c).l ){
      if( *ip >= (*c).l )
	if( *ip <= (*c).u )
	  printf( "WHITELIST: %s\n", inet_ntoa( ip_addr ) );
      c++;
    }
   
    if( ip == work->end ){
      printf( "QUITING TIME\n");
      ip = NULL;
      break;
    }

    ip++;

  }
  return NULL;
}

void printIPArray( unsigned long * ipArray ){
  return;
  unsigned long * ip = ipArray;
  while( ip ){
    printf( "%u\n", *ip );
    ip++;
  }
}

int main( int argc, char ** argv ){

  /* Call the initializer function, which will
   * populate our data structure for us, and we
   * just search the datastructure, looking for
   * each unsigned integer within the data struct
   */
  char * filename = NULL;
  int nthreads = 1;

  if( argc < 2 ){
    perror( "You must supply a filename as a CIDR whitelist input" );
    filename = "whitelist";
  }
  else {
    filename = argv[1];
  }

  if( argc >= 2 ){
    nthreads = atoi( argv[2] );
  }

  cidr_pair_t * cp = NULL;
  unsigned long *ipArray;
  int nips;

  cp = readCIDRFromFile( filename );
  ipArray = readIPsFromStdIn( &nips );

  printIPArray(ipArray);

  printf("Whitelist start address is: %x\n", (void *)cp );

  pthread_attr_t attr;

  int rc = pthread_attr_init(&attr);

  if ( rc != 0 )
    perror("pthread_attr_init");

  int i;
  pthread_t threadid;
  pthread_t *threads = (pthread_t *)malloc( sizeof(pthread_t) * nthreads );

  int units = nips / nthreads;

  printf("Number of worker threads: %d\n", nthreads );
  printf("Number of IPs to work on: %d\n", nips );
  printf("Units of work per thread: %d\n", units );

  unsigned long * startptr = ipArray;
  unsigned long * endptr   = startptr + (units * sizeof( unsigned long *)) ;

  printf("Delta: %d\n", ( endptr - startptr ) / sizeof( unsigned long *));

  int unitsleft = nips - units;

  for( i = 0; i < nthreads; i++ ){
    /* We need to split the data up into equal chunks across
     * the number of threads. Create a datastructure for each
     * thread, and pass in the pointer to the start piece of
     * work, and the end piece of work, and a pointer to the
     * whitelist CIDR pair structure.
     */

    work_data_t *work = (work_data_t*)malloc( sizeof( work_data_t ) );
    work->whitelist = cp;

    // Iterate through the values, stopping either at count of
    // units, or a value of 0
    int k;
    endptr = startptr;
    for( k = 0; k < units; k++ ){
      if( *endptr ){
	endptr++;
	unitsleft--;
      }
    }

    work->start = startptr;
    work->end   = endptr;
    work->id    = i;
    
    rc = pthread_create( &threadid,
			 &attr,
			 &thread_start,
			 work );
    threads[i] = threadid;

    printf("Thread created, %d units left\n", (unitsleft < 0? 0 : unitsleft) );

    if( *endptr ){
      startptr = endptr;
      startptr++;
    }

    printf("Delta: %d\n", ( endptr - startptr ) / sizeof( unsigned long *));
    
  }

  pthread_attr_destroy(&attr);

  /* wait for ALL of the work to complete */
  
  for( i = 0; i < nthreads; i++ ){
    pthread_join(threads[i], NULL);
    printf("Joined a thread\n");
  }

  printf("All threads joined\n");

  return 0;
}
