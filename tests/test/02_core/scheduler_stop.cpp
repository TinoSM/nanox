/*************************************************************************************/
/*      Copyright 2015 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

/*
<testinfo>
test_generator=gens/core-generator
test_generator_ENV=( "NX_TEST_SCHEDULE=wf" )
</testinfo>
*/

#include "config.hpp"
#include "nanos.h"
#include "atomic.hpp"
#include <iostream>
#include "smpprocessor.hpp"
#include "system.hpp"

using namespace std;

using namespace nanos;
using namespace nanos::ext;

#define NUM_ITERS     1000
#define NUM_RUNS   100

Atomic<int> A;

typedef struct {
   nanos_loop_info_t loop_info;
} main__loop_1_data_t;

void main__loop_1 ( void *args );

void main__loop_1 ( void *args )
{
   A++;
}


int main ( int argc, char **argv )
{
   int i;
   bool check = true;

   main__loop_1_data_t _loop_data;

   // Repeat the test NUM_RUNS times
   for ( int testNumber = 0; testNumber < NUM_RUNS; ++testNumber ) {
      A = 0;
   
      WD *wg = getMyThreadSafe()->getCurrentWD();   
      // Stop scheduler
      sys.stopScheduler();
      // increment variable
      for ( i = 0; i < NUM_ITERS; i++ ) {
         // Work descriptor creation
         WD * wd = new WD( new SMPDD( main__loop_1 ), sizeof( _loop_data ), __alignof__(nanos_loop_info_t), ( void * ) &_loop_data );
         wd->setPriority( 100 );
   
         // Work Group affiliation
         wg->addWork( *wd );
   
         // Work submission
         sys.submit( *wd );
      }
      // Re-enable the scheduler
      sys.startScheduler();
      // barrier (kind of)
      wg->waitCompletion();
      
   
      /*
       * The verification criteria is that A is equal to the number of tasks
       * run. Should A be lower, that would indicate that not all tasks
       * successfuly finished.
       */
      if ( A.value() != NUM_ITERS ) check = false;
   }

   if ( check ) {
      fprintf(stderr, "%s : %s\n", argv[0], "successful");
      return 0;
   }
   else {
      fprintf(stderr, "%s: %s\n", argv[0], "unsuccessful");
      return -1;
   }
}

