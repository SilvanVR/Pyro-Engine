#include "time.h"

#include "logger/logger.h"
#include "utils/utils.h"

namespace Pyro
{

    // ID-Generator for function-objects attached to the Time-System
    static IDGenerator<CallbackID> idGenerator;

    //---------------------------------------------------------------------------
    //  Static Member Initilization
    //---------------------------------------------------------------------------

    uint64_t        Time::delta = 0;
    Timer           Time::globalTimer;
    int             Time::fpsCap = 0;
    unsigned int    Time::fps = 0;

    const uint64_t  Time::MILLISECOND = 1000000;
    const uint64_t  Time::SECOND = 1000000000;

    std::vector<CallbackTimer> Time::timerCallbacks;

    //---------------------------------------------------------------------------
    //  Static Member Functions
    //---------------------------------------------------------------------------

    // Returns the delta between the last two updates of the TimeManager class in ns.
    uint64_t Time::getDelta()
    {
        return delta;
    }

    // Returns the delta between the last two updates of the TimeManager class in sec.
    double Time::getDeltaSeconds()
    {
        return static_cast<double>(delta) / SECOND;
    }

    // Returns the frames per second. Updated every second.
    unsigned int Time::getFPS()
    {
        return fps;
    }

    // Return the total running time of the application in ns
    uint64_t Time::getNanoTime()
    {
        return globalTimer.elapsedPrecise();
    }

    // Returns the total running time of the application in ms.
    double Time::getTotalRunningTime()
    {
        return globalTimer.elapsedMillis();
    }

    // Returns the total running time of the application in seconds.
    double Time::getTotalRunningTimeInSeconds()
    {
        return globalTimer.elapsedSeconds();
    }

    CallbackID Time::setInterval(const std::function<void()>& func, uint64_t ms)
    {
        CallbackInfo info;
        info.callback = func;
        info.id = idGenerator.generateID();

        timerCallbacks.push_back(CallbackTimer(info, ms, false));

        return info.id;
    }

    CallbackID Time::setTimeout(const std::function<void()>& func, uint64_t ms)
    {
        CallbackInfo info;
        info.callback = func;
        info.id = idGenerator.generateID();

        timerCallbacks.push_back(CallbackTimer(info, ms, true));

        return info.id;
    }

    void Time::clearCallback(CallbackID id)
    {
        if (id == INVALID_CALLBACK_ID)
            return;

        timerCallbacks.erase(std::remove_if(timerCallbacks.begin(), timerCallbacks.end(),
                             [=](const CallbackTimer& timer) { return id == timer.getID(); }), timerCallbacks.end());
        idGenerator.freeID(id);
    }

}