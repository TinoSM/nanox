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

#include "system.hpp"
#include "throttle.hpp"
#include "plugin.hpp"
#include "config.hpp"


namespace nanos {
   namespace ext {

      class ReadyTasksThrottle : public ThrottlePolicy
      {
         private:
            int _limit;

            ReadyTasksThrottle ( const ReadyTasksThrottle & );
            const ReadyTasksThrottle operator= ( const ReadyTasksThrottle & );
            
         public:
            //must be public: used in the plugin
            static const int _defaultLimit;

            ReadyTasksThrottle( int actualLimit = _defaultLimit ) : _limit ( actualLimit ) {}

            void setLimit( int mr ) { _limit = mr; }

            bool throttle();

            ~ReadyTasksThrottle() {}
      };

      const int ReadyTasksThrottle::_defaultLimit = 100;

      bool ReadyTasksThrottle::throttle()
      {
         //checking if the number of ready tasks is higher than the allowed maximum
         if ( sys.getReadyNum() > _limit )  {
            verbose0( "Throttle Policy: avoiding task creation!" );
            return false;
         }

         return true;
      }

      //factory
      static ReadyTasksThrottle * createReadyTasksThrottle( int actualLimit )
      {
         return new ReadyTasksThrottle( actualLimit );
      }

      class ReadyTasksThrottlePlugin : public Plugin
      {

         public:
            ReadyTasksThrottlePlugin() : Plugin( "Ready Task Throttle Plugin",1 ) {}

            virtual void init() {
               Config config;

               int actualLimit = ReadyTasksThrottle::_defaultLimit; 
               config.registerArgOption( new Config::PositiveVar( "throttle-limit", 
                                          actualLimit ) ); 
               config.init(); 
               sys.setThrottlePolicy( createReadyTasksThrottle( actualLimit )); 
            }
      };

   }
}

nanos::ext::ReadyTasksThrottlePlugin NanosXPlugin;
