#include "timer.h"

#include "time.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Timer Class - Public Functions
    //---------------------------------------------------------------------------

    float Timer::elapsed() const 
    { 
        return static_cast<float>(m_elapsed) / Time::MILLISECOND; 
    }

    // Return the elapsed time as a double in milliseconds
    double Timer::elapsedMillis() const 
    { 
        return static_cast<double>(m_elapsed) / Time::MILLISECOND; 
    }

    // Return the elapsed time as a double in seconds
    double Timer::elapsedSeconds() const 
    { 
        return static_cast<double>(m_elapsed) / Time::SECOND; 
    }

    //---------------------------------------------------------------------------
    //  CallbackTimer Class - Constructor
    //---------------------------------------------------------------------------

    CallbackTimer::CallbackTimer(const CallbackInfo& cbInfo, uint64_t ms, bool repeatOnlyOnce)
        : callbackInfo(cbInfo), duration(ms * Time::MILLISECOND), repeatOnce(repeatOnlyOnce), finished(false) 
    {}

    //---------------------------------------------------------------------------
    //  CallbackTimer Class - Public Functions
    //---------------------------------------------------------------------------

    void CallbackTimer::update(uint64_t nanos)
    {
        Timer::update(nanos);
        if (m_elapsed > duration)
        {
            callbackInfo.callback();
            m_elapsed -= duration;
            if (repeatOnce) finished = true;
        }
    }



}