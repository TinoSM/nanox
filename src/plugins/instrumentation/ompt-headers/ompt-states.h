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

#ifndef OMPT_STATES_H_INCLUDED
#define OMPT_STATES_H_INCLUDED

typedef enum {
  /* work states (0..15) */
  ompt_state_work_serial            = 0x00, /* working outside parallel   */
  ompt_state_work_parallel          = 0x01, /* working within parallel    */
  ompt_state_work_reduction         = 0x02, /* performing a reduction     */

  /* idle (16..31) */
  ompt_state_idle                   = 0x10, /* waiting for work           */

  /* overhead states (32..63) */
  ompt_state_overhead               = 0x20, /* non-wait overhead          */

  /* barrier wait states (64..79) */
  ompt_state_wait_barrier           = 0x40, /* generic barrier            */
  ompt_state_wait_barrier_implicit  = 0x41, /* implicit barrier           */
  ompt_state_wait_barrier_explicit  = 0x42, /* explicit barrier           */
    
 /* task wait states (80..95) */
  ompt_state_wait_taskwait          = 0x50, /* waiting at a taskwait      */
  ompt_state_wait_taskgroup         = 0x51, /* waiting at a taskgroup     */
	
  /* mutex wait states (96..111) */	      	    
  ompt_state_wait_lock              = 0x60, /* waiting for lock           */
  ompt_state_wait_nest_lock         = 0x61, /* waiting for nest lock      */
  ompt_state_wait_critical          = 0x62, /* waiting for critical       */
  ompt_state_wait_atomic            = 0x63, /* waiting for atomic         */
  ompt_state_wait_ordered           = 0x64, /* waiting for ordered        */
  
  /* target (112..127) */
  ompt_state_wait_target            = 0x70, /* waiting for target         */ 
  ompt_state_wait_target_data       = 0x71, /* waiting for target data    */ 
  ompt_state_wait_target_update     = 0x72, /* waiting for target update  */ 

  /* misc (128..143) */
  ompt_state_undefined              = 0x80, /* undefined thread state     */
  ompt_state_first                  = 0x81, /* initial enumeration state  */
} ompt_state_t;

#endif /* OMPT_STATES_H_INCLUDED */
