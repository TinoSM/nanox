/**************************************************************************/
/*      Copyright 2010 Barcelona Supercomputing Center                    */
/*      Copyright 2009 Barcelona Supercomputing Center                    */
/*                                                                        */
/*      This file is part of the NANOS++ library.                         */
/*                                                                        */
/*      NANOS++ is free software: you can redistribute it and/or modify   */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or  */
/*      (at your option) any later version.                               */
/*                                                                        */
/*      NANOS++ is distributed in the hope that it will be useful,        */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     */
/*      GNU Lesser General Public License for more details.               */
/*                                                                        */
/*      You should have received a copy of the GNU Lesser General Public License  */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.  */
/**************************************************************************/

#include "os.hpp"
#include "smpprocessor.hpp"
#include "schedule.hpp"
#include "debug.hpp"
#include "system.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include "smp_ult.hpp"
#include "instrumentation.hpp"
//<<<<<<< HEAD
#if 0
//#include "clusterdevice_decl.hpp"
#include "resourcemanager.hpp"
#include "taskexecutionexception_decl.hpp"
#endif
//=======
//>>>>>>> master

using namespace nanos;
using namespace nanos::ext;

//<<<<<<< HEAD
#if 0
void * smp_bootthread ( void *arg )
{
   SMPThread *self = static_cast<SMPThread *>( arg );
#ifdef NANOS_RESILIENCY_ENABLED
   self->setupSignalHandlers();
#endif

   self->run();

   NANOS_INSTRUMENT ( static InstrumentationDictionary *ID = sys.getInstrumentation()->getInstrumentationDictionary(); )
   NANOS_INSTRUMENT ( static nanos_event_key_t cpuid_key = ID->getEventKey("cpuid"); )
   NANOS_INSTRUMENT ( nanos_event_value_t cpuid_value =  (nanos_event_value_t) 0; )
   NANOS_INSTRUMENT ( sys.getInstrumentation()->raisePointEvents(1, &cpuid_key, &cpuid_value); )


   self->BaseThread::finish();
   pthread_exit ( 0 );

   // We should never get here!
   return NULL;
}

// TODO: detect at configure
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

void SMPThread::start ()
{
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   // user-defined stack size
   if ( _stackSize > 0 ) {
     if ( _stackSize < PTHREAD_STACK_MIN ) {
       warning("specified thread stack too small, adjusting it to minimum size");
       _stackSize = PTHREAD_STACK_MIN;
     }

     if (pthread_attr_setstacksize( &attr, _stackSize ) )
       warning("couldn't set pthread stack size stack");
   }

   if ( pthread_create( &_pth, &attr, smp_bootthread, this ) )
      fatal( "couldn't create thread" );

   if ( pthread_cond_init( &_condWait, NULL ) < 0 )
      fatal( "couldn't create pthread condition wait" );

   if ( pthread_mutex_init( &_mutexWait, NULL ) < 0 )
      fatal( "couldn't create pthread mutex wait" );
}

void SMPThread::finish ()
{
   if ( pthread_cond_destroy( &_condWait ) < 0 )
      fatal( "couldn't destroy pthread condition wait" );
}
#endif
//=======
//>>>>>>> master

void SMPThread::runDependent ()
{
   WD &work = getThreadWD();
   setCurrentWD( work );

   SMPDD &dd = ( SMPDD & ) work.activateDevice( SMP );

   dd.execute( work );
}

void SMPThread::idle( bool debug )
{
   if ( sys.getNetwork()->getNumNodes() > 1 ) {
      sys.getNetwork()->poll(0);

      if ( !_pendingRequests.empty() ) {
         std::set<void *>::iterator it = _pendingRequests.begin();
         while ( it != _pendingRequests.end() ) {
            GetRequest *req = (GetRequest *) (*it);
            if ( req->isCompleted() ) {
               std::set<void *>::iterator toBeDeletedIt = it;
               it++;
               _pendingRequests.erase(toBeDeletedIt);
               req->clear();
               delete req;
            } else {
               it++;
            }
         }
      }
   }
}

void SMPThread::wait()
{
   NANOS_INSTRUMENT ( static InstrumentationDictionary *ID = sys.getInstrumentation()->getInstrumentationDictionary(); )
   NANOS_INSTRUMENT ( static nanos_event_key_t cpuid_key = ID->getEventKey("cpuid"); )
   NANOS_INSTRUMENT ( nanos_event_value_t cpuid_value = (nanos_event_value_t) 0; )
   NANOS_INSTRUMENT ( sys.getInstrumentation()->raisePointEvents(1, &cpuid_key, &cpuid_value); )

   ThreadTeam *team = getTeam();

   /* Put WD's from local to global queue, leave team, and set flag
    * Locking _mutexWait assures us that the sleep flag is still set when we get here
    */
   lock();
   _pthread.mutex_lock();

   if ( isSleeping() ) {

      if ( hasNextWD() ) {
         WD *next = getNextWD();
         next->untie();
         team->getSchedulePolicy().queue( this, *next );
      }
      fatal_cond( hasNextWD(), "Can't sleep a thread with more than 1 WD in its local queue" );

      if ( team != NULL ) leaveTeam();
      BaseThread::wait();

      unlock();
//<<<<<<< HEAD
//=======
      _pthread.cond_wait();
//>>>>>>> master

      NANOS_INSTRUMENT( InstrumentState state_stop(NANOS_STOPPED) );

      /* Wait on condition loop */
      while ( isSleeping() ) {
         pthread_cond_wait( &_condWait, &_mutexWait );
      }
      pthread_mutex_unlock( &_mutexWait );

      NANOS_INSTRUMENT( InstrumentState state_wake(NANOS_WAKINGUP) );

      /* Set waiting status flag */
      lock();
      BaseThread::resume();
      unlock();

//<<<<<<< HEAD
      /* Whether the thread should wait for the cpu to be free before doing some work */
#if 0
      ResourceManager::waitForCpuAvailability();
#endif
//=======
   _pthread.mutex_unlock();
//>>>>>>> master

      if ( isSleeping() ) wait();
      else {
         NANOS_INSTRUMENT ( if ( sys.getBinding() ) { cpuid_value = (nanos_event_value_t) getCpuId() + 1; } )
         NANOS_INSTRUMENT ( if ( !sys.getBinding() && sys.isCpuidEventEnabled() ) { cpuid_value = (nanos_event_value_t) sched_getcpu() + 1; } )
         NANOS_INSTRUMENT ( sys.getInstrumentation()->raisePointEvents(1, &cpuid_key, &cpuid_value); )
      }
   }
   else {
      pthread_mutex_unlock( &_mutexWait );
      unlock();
   }
}

void SMPThread::wakeup()
{
//<<<<<<< HEAD
#if 0
   pthread_mutex_lock( &_mutexWait );
   BaseThread::wakeup();
   pthread_cond_signal( &_condWait );
   pthread_mutex_unlock( &_mutexWait );
}

void SMPThread::sleep()
{
   pthread_mutex_lock( &_mutexWait );
   BaseThread::sleep();
   pthread_mutex_unlock( &_mutexWait );
}

void SMPThread::block()
{
   pthread_mutex_lock( &_completionMutex );
   pthread_cond_wait( &_completionWait, &_completionMutex );
   pthread_mutex_unlock( &_completionMutex );
#endif
//=======
   //! \note This function has to be in free race condition environment or externally
   // protected, when called, with the thread common lock: lock() & unlock() functions.

   //! \note If thread is not marked as waiting, just ignore wakeup
   if ( !isSleeping() || !isWaiting() ) return;

   _pthread.wakeup();
//>>>>>>> master
}


// This is executed in between switching stacks
void SMPThread::switchHelperDependent ( WD *oldWD, WD *newWD, void *oldState  )
{
   SMPDD & dd = ( SMPDD & )oldWD->getActiveDevice();
   dd.setState( (intptr_t *) oldState );
}

bool SMPThread::inlineWorkDependent ( WD &wd )
{
   // Now the WD will be inminently run
   wd.start(WD::IsNotAUserLevelThread);

   SMPDD &dd = ( SMPDD & )wd.getActiveDevice();

   NANOS_INSTRUMENT ( static nanos_event_key_t key = sys.getInstrumentation()->getInstrumentationDictionary()->getEventKey("user-code") );
   NANOS_INSTRUMENT ( nanos_event_value_t val = wd.getId() );
   NANOS_INSTRUMENT ( sys.getInstrumentation()->raiseOpenStateAndBurst ( NANOS_RUNNING, key, val ) );

   //if ( sys.getNetwork()->getNodeNum() > 0 ) std::cerr << "Starting wd " << wd.getId() << std::endl;
   
   dd.execute( wd );

   NANOS_INSTRUMENT ( sys.getInstrumentation()->raiseCloseStateAndBurst ( key, val ) );
   return true;
}

void SMPThread::switchTo ( WD *wd, SchedulerHelper *helper )
{
   // wd MUST have an active SMP Device when it gets here
   ensure( wd->hasActiveDevice(),"WD has no active SMP device" );
   SMPDD &dd = ( SMPDD & )wd->getActiveDevice();
   ensure( dd.hasStack(), "DD has no stack for ULT");

   ::switchStacks(
       ( void * ) getCurrentWD(),
       ( void * ) wd,
       ( void * ) dd.getState(),
       ( void * ) helper );
}

void SMPThread::exitTo ( WD *wd, SchedulerHelper *helper)
{
   // wd MUST have an active SMP Device when it gets here
   ensure( wd->hasActiveDevice(),"WD has no active SMP device" );
   SMPDD &dd = ( SMPDD & )wd->getActiveDevice();
   ensure( dd.hasStack(), "DD has no stack for ULT");

   //TODO: optimize... we don't really need to save a context in this case
   ::switchStacks(
      ( void * ) getCurrentWD(),
      ( void * ) wd,
      ( void * ) dd.getState(),
      ( void * ) helper );
}

int SMPThread::getCpuId() const {
   return _core->getBindingId();
}

SMPMultiThread::SMPMultiThread( WD &w, SMPProcessor *pe, unsigned int representingPEsCount, PE **representingPEs ) : SMPThread ( w, pe, pe ), _current( 0 ), _totalThreads( representingPEsCount ) {
   setCurrentWD( w );
   _threads.reserve( representingPEsCount );
   for ( unsigned int i = 0; i < representingPEsCount; i++ )
   {
      _threads[ i ] = &( representingPEs[ i ]->startWorker( this ) );
   }
}
