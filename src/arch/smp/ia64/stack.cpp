#include "smpprocessor.hpp"

using namespace nanos;


/* -------------------------------------------------------------------
 * Initial STACK state for IA-64
 * -------------------------------------------------------------------
 *
 *  +------------------------------+
 *  | (00) f31                     |<-[context]
 *  | (02) f30                     |
 *  | (..)                         |
 *  | (30) f16                     |
 *  | (32) f5                      |
 *  | (..)                         |
 *  | (38) f2                      |
 *  | (40) ar.fpsr                 |
 *  | (41) b5                      |
 *  | (..)                         |
 *  | (45) b1                      |
 *  | (46) ar.lc                   |
 *  | (47) pr                      |
 *  | (48) ar.unat(callee)         |
 *  | (49) r7 == 'user func'       |
 *  | (50) r6 == 'cleanup arg'     |
 *  | (51) r5 == 'cleanup'         |
 *  | (52) r4                      |
 *  | (53) ar.unat(caller)         |
 *  | (54) tp (r13)                |
 *  | (55) ar.bspstore             |
 *  | (56) ar.rnat                 |
 *  | (57) ar.pfs                  |
 *  | (58) ar.rsc                  |
 *  | (59) rp == 'startHelper'     |
 *  +------------------------------+
 *  | (60) arg                     |
 *  +------------------------------+
 *
 * -----------------------------------------------------------------*/

extern "C" {
// low-level helper routine to start a new user-thread
void startHelper ();
}

void SMPDD::initStackDep ( void *userfuction, void *data, void *cleanup )
{
   // stack grows down
   state = stack;
   state += stackSize;

   state -= 68;

   // argument
   state[60] = (intptr_t) data;

   // return pointer
   state[59] = (intptr_t) *((long *)startHelper);
   // ar.rsc
   state[58] = (intptr_t) 3; 
   // ar.pfs
   state[57] = (intptr_t) 0;
   // ar.rnat
   state[56] = (intptr_t) 0;
   // ar.bspstore
   state[55] = (intptr_t) stack;
   // tp(r13)
   state[54] = (intptr_t) 0; 
   // ar.unat (caller)
   state[53] = (intptr_t) 0; 
   // r5 (cleanup)
   state[51] = (intptr_t) cleanup; 
   // r6 (pt)
   state[50] = (intptr_t) this; 
   // r7 (userf)
   state[49] = (intptr_t) userfuction;
   // ar.unat (callee)
   state[48] = (intptr_t) 0;
   // ar.fpsr
   state[40] = (intptr_t) 0x9804c0270033f;
   
}
