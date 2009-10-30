/*************************************************************************************/
/*      Copyright 2009 Barcelona Supercomputing Center                               */
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

#ifndef _NANOS_SMP_THREAD
#define _NANOS_SMP_THREAD

#include "smpdd.hpp"
#include "basethread.hpp"
#include <pthread.h>

//TODO: Make smp independent from pthreads? move it to OS?

namespace nanos
{

   class SMPThread : public BaseThread
   {

         friend class SMPProcessor;

      private:
         pthread_t   pth;
         bool        useUserThreads;

         // disable copy constructor and assignment operator
         SMPThread( const SMPThread &th );
         const SMPThread & operator= ( const SMPThread &th );

      public:
         // constructor
         SMPThread( WD &w, PE *pe, bool uUT=true ) : BaseThread( w,pe ),useUserThreads( uUT ) {}

         // destructor

         virtual ~SMPThread() { if ( isStarted() ) /*TODO: stop()*/; }

         void setUseUserThreads( bool value=true ) { useUserThreads = value; }

         virtual void start();
         virtual void join();
         virtual void run_dependent ( void );

         virtual void inlineWork( WD *work );
         virtual void switchTo( WD *work );
         virtual void exitTo( WD *work );
         virtual void bind( void );
   };


};

#endif