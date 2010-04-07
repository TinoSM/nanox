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

#include "nanos.h"
#include "basethread.hpp"
#include "debug.hpp"
#include "system.hpp"
#include "workdescriptor.hpp"
#include "smpdd.hpp"
#include "gpudd.hpp"
#include "plugin.hpp"
#include "instrumentor.hpp"

using namespace nanos;

// TODO: move to dependent part
const size_t nanos_smp_dd_size = sizeof(ext::SMPDD);
const size_t nanos_gpu_dd_size = sizeof(ext::GPUDD);

void * nanos_gpu_factory( void *prealloc, void *args )
{
   nanos_smp_args_t *smp = ( nanos_smp_args_t * ) args;
   if ( prealloc != NULL )
   {
      return ( void * )new (prealloc) ext::GPUDD( smp->outline );
   }
   else
   {
      return ( void * ) new ext::GPUDD( smp->outline );
   }
}

void * nanos_smp_factory( void *prealloc, void *args )
{
   nanos_smp_args_t *smp = ( nanos_smp_args_t * ) args;

   if ( prealloc != NULL )
   {
      return ( void * )new (prealloc) ext::SMPDD( smp->outline );
   }
   else 
   {
      return ( void * )new ext::SMPDD( smp->outline );
   }
}

nanos_wd_t nanos_current_wd()
{
   NANOS_INSTRUMENTOR( enterRuntimeAPI(CURRENT_WD,RUNTIME) )
   nanos_wd_t cwd = myThread->getCurrentWD();
   NANOS_INSTRUMENTOR( leaveRuntimeAPI() )
   return cwd;
}

int nanos_get_wd_id ( nanos_wd_t wd )
{
   NANOS_INSTRUMENTOR( enterRuntimeAPI(GET_WD_ID, RUNTIME) );
   WD *lwd = ( WD * )wd;
   int id = lwd->getId();
   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return id;
}

/*! \brief Creates a new WorkDescriptor
 *
 *  \sa nanos::WorkDescriptor
 */
nanos_err_t nanos_create_wd (  nanos_wd_t *uwd, size_t num_devices, nanos_device_t *devices, size_t data_size,
                               void ** data, nanos_wg_t uwg, nanos_wd_props_t *props, size_t num_copies, nanos_copy_data_t **copies )
{
   try 
   {
      NANOS_INSTRUMENTOR( enterRuntimeAPI(CREATE_WD,RUNTIME) );
      if ( ( props == NULL  || ( props != NULL  && !props->mandatory_creation ) ) && !sys.throttleTask() ) {
         *uwd = 0;
         return NANOS_OK;
      }
      sys.createWD ( (WD **) uwd, num_devices, devices, data_size, (void **) data, (WG *) uwg, props, num_copies, copies );

   } catch ( ... ) {
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

/*! \brief Creates a new Sliced WorkDescriptor
 *
 *  \sa nanos::WorkDescriptor
 */
nanos_err_t nanos_create_sliced_wd ( nanos_wd_t *uwd, size_t num_devices, nanos_device_t *devices, size_t outline_data_size,
                               void ** outline_data, nanos_wg_t uwg, nanos_slicer_t slicer, size_t slicer_data_size,
                               nanos_slicer_data_t * slicer_data, nanos_wd_props_t *props, size_t num_copies, nanos_copy_data_t **copies )
{
   try 
   {
      NANOS_INSTRUMENTOR( enterRuntimeAPI(CREATE_WD,RUNTIME) );
      if ( ( props == NULL  || ( props != NULL  && !props->mandatory_creation ) ) && !sys.throttleTask() ) {
         *uwd = 0;
         return NANOS_OK;
      }
      if ( slicer_data == NULL ) {
         NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
         return NANOS_UNKNOWN_ERR;
      }

      sys.createSlicedWD ( (WD **) uwd, num_devices, devices, outline_data_size, outline_data, (WG *) uwg,
                           (Slicer *) slicer, slicer_data_size, (SlicerData *&) *slicer_data, props, num_copies, copies );

   } catch ( ... ) {
      // xteruel:FIXME: Will be interesting to instrument new wd info: (WD *) *uwd
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

nanos_err_t nanos_submit ( nanos_wd_t uwd, size_t num_deps, nanos_dependence_t *deps, nanos_team_t team )
{
   try {
      // xteruel:FIXME: Will be interesting to instrument new wd info: (WD *) *uwd
      NANOS_INSTRUMENTOR( enterRuntimeAPI(SUBMIT_WD, RUNTIME) );
      ensure( uwd,"NULL WD received" );

      WD * wd = ( WD * ) uwd;

      if ( team != NULL ) {
         warning( "Submitting to another team not implemented yet" );
      }

      if ( deps != NULL ) {
         sys.submitWithDependencies( *wd, num_deps, deps );
         return NANOS_OK;
      }

      sys.submit( *wd );
   } catch ( ... ) {
      // xteruel:FIXME: Will be interesting to instrument new wd info: (WD *) *uwd
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   // xteruel:FIXME: Will be interesting to instrument new wd info: (WD *) *uwd
   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}


// data must be not null
nanos_err_t nanos_create_wd_and_run ( size_t num_devices, nanos_device_t *devices, size_t data_size, void * data, 
                                      size_t num_deps, nanos_dependence_t *deps, nanos_wd_props_t *props,
                                      size_t num_copies, nanos_copy_data_t *copies )
{
   try {
      NANOS_INSTRUMENTOR( enterRuntimeAPI(CREATE_WD_AND_RUN, RUNTIME) );
      if ( num_devices > 1 ) warning( "Multiple devices not yet supported. Using first one" );

      // TODO: choose device
      // pre-allocate device
      char chunk[devices[0].dd_size];

      WD wd( ( DD* ) devices[0].factory( chunk, devices[0].arg ), data_size, data, num_copies, copies );

      if ( deps != NULL ) {
         sys.waitOn( num_deps, deps );
      }

      sys.inlineWork( wd );

   } catch ( ... ) {
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

nanos_err_t nanos_set_internal_wd_data ( nanos_wd_t wd, void *data )
{
   try {
      NANOS_INSTRUMENTOR( enterRuntimeAPI(SET_INTERNAL_WD_DATA,RUNTIME) );
      WD *lwd = ( WD * ) wd;

      lwd->setInternalData( data );
   } catch ( ... ) {
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

nanos_err_t nanos_get_internal_wd_data ( nanos_wd_t wd, void **data )
{
   try {
      NANOS_INSTRUMENTOR( enterRuntimeAPI(GET_INTERNAL_WD_DATA,RUNTIME) );
      WD *lwd = ( WD * ) wd;
      void *ldata;

      ldata = lwd->getInternalData();

      *data = ldata;
   } catch ( ... ) {
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

nanos_err_t nanos_yield ( void )
{
   try {
      NANOS_INSTRUMENTOR( enterRuntimeAPI( YIELD,RUNTIME) );
      Scheduler::yield();

   } catch ( ... ) {
      NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
      return NANOS_UNKNOWN_ERR;
   }

   NANOS_INSTRUMENTOR( leaveRuntimeAPI() );
   return NANOS_OK;
}

