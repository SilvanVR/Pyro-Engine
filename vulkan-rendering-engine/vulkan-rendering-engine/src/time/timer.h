#ifndef TIMER_H_
#define TIMER_H_

// Contains the classes "Timer" + "CallbackTimer"

#include <stdint.h>
#include "structs.hpp"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Timer Class
    //---------------------------------------------------------------------------

    class Timer
    {
    public:
        Timer() : m_elapsed(0), m_paused(false) {}
        virtual ~Timer() {}

        virtual void    update(uint64_t nanos) { if (!m_paused) m_elapsed += nanos; }
        void            reset() { m_elapsed = 0; }
        void            pause() { m_paused = true; }

        // Return the elapsed time in nanoseconds from this timer
        uint64_t    elapsedPrecise() const { return m_elapsed; }

        // Return the elapsed time as a float in milliseconds
        float       elapsed() const;

        // Return the elapsed time as a double in milliseconds
        double      elapsedMillis() const;

        // Return the elapsed time as a double in seconds
        double      elapsedSeconds() const;

    protected:
        uint64_t    m_elapsed;
        bool        m_paused;
    };

    //---------------------------------------------------------------------------
    //  CallbackTimer Class
    //---------------------------------------------------------------------------


    class CallbackTimer : public Timer
    {
    public:
        CallbackTimer(const CallbackInfo& cbInfo, uint64_t ms, bool repeatOnlyOnce);
        ~CallbackTimer() {}

        void            update(uint64_t nanos) override;
        void            setIsFinished(bool b) { finished = b; }
        unsigned char   getID()         const { return callbackInfo.id; }
        bool            isFinished()    const { return finished; }

        bool operator==(const CallbackTimer& other) { return callbackInfo.id == other.callbackInfo.id; }

    private:
        CallbackInfo    callbackInfo;
        uint64_t        duration;
        bool            repeatOnce;
        bool            finished;
    };





}



#endif // !TIMER_H_

