/*
*  Time - Class header file.
*  Contains lots of information about the time of the Application.
*  Fields are updated by the TimeManager - class.
*
*  Date:    22.04.2016
*  Creator: Silvan Hau
*/

#ifndef TIME_H_
#define TIME_H_

#include "timer.h"
#include <stdint.h>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Time Class
    //---------------------------------------------------------------------------

    class Time
    {
        friend class TimeManager; // Allow the TimeManager-Class access to the private fields

    public:
        static uint64_t         getDelta();
        static double           getDeltaSeconds();
        static unsigned int     getFPS();
        static uint64_t         getNanoTime();
        static double           getTotalRunningTime();
        static double           getTotalRunningTimeInSeconds();
        static void             capFPS(int _fpsCap) { fpsCap = _fpsCap; }
        static uint32_t         numTimerCallbacks(){ return static_cast<uint32_t>(timerCallbacks.size()); }

        // Call the given function every x-milliseconds
        static CallbackID       setInterval(const std::function<void()>& func, uint64_t ms);

        // Call the given function after x-milliseconds once
        static CallbackID       setTimeout(const std::function<void()>& func, uint64_t ms);
        
        // Clear a callback by a given id. The id itself is returned along "setInterval()" or "setTimeout()"
        static void             clearCallback(CallbackID id);

        static const uint64_t   MILLISECOND;
        static const uint64_t   SECOND;

    private:
        static uint64_t         delta;              // Time in ns between two frames
        static unsigned int     fps;                // Current Frames Per Second
        static Timer            globalTimer;        // Timer which measures total running time
        static int              fpsCap;             // FPS-Cap

        // Stores the timer-objects created along with "setInterval" + "setTimeout"
        static std::vector<CallbackTimer> timerCallbacks;
    };


}

#endif // !TIME_H_
