/*
*  TimeManager - Class header file.
*  Handles and updates the Time-Class, which contains information about running time, delta time etc.
*  update() should be called at the beginning of each tick in the main loop.
*
*  Date:    22.04.2016
*  Creator: Silvan Hau
*/

#ifndef TIME_MANAGER_H_
#define TIME_MANAGER_H_

#include "time.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  TimeManager class
    //---------------------------------------------------------------------------

    class TimeManager
    {

    public:
        static void update(); // Update the time manager - should be called at the beginning of the main loop 

        static std::string getCurrentDate(); // Return the current date in a format like this: Mon Jul 29 08:41:09 2013

    private:
        static void updateCallbackTimer(uint64_t delta);

    };

}


#endif // !TIME_MANAGER_H_