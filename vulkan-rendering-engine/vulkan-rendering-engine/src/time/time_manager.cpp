#include "time_manager.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Public Static Functions
    //---------------------------------------------------------------------------

    std::string TimeManager::getCurrentDate()
    {
        auto currentTime = std::chrono::system_clock::now();
        std::time_t dateTime = std::chrono::system_clock::to_time_t(currentTime);
        char* date = std::ctime(&dateTime);

        // cut off "\n" character
        date[strlen(date) - 1] = '\0';

        return date;
    }

    //---------------------------------------------------------------------------
    //  Static Member Functions
    //---------------------------------------------------------------------------

    //Update function should be called at the beginning of each tick in the main loop, to update all the related fields in the Time-Class.
    void TimeManager::update()
    {
        using namespace std::chrono;
        using Clock = high_resolution_clock;

        static auto         lastFrameTime = Clock::now();   // Last frame time
        static uint64_t     fpsTimer = 0;                   // fps timer (one second interval) which updates the fps in the Time-Class
        static unsigned int frameCounter = 0;               // Counts the actual frames (increments every update and resets every second)

        if (Time::fpsCap > 0)
        {
            // Delta-Time in NS
            uint64_t deltaTimeNS = static_cast<uint64_t>((1.0f / Time::fpsCap) * Time::SECOND);

            // How long it took to update and render
            auto currentTime = Clock::now();
            auto deltaTime = currentTime - lastFrameTime;
            uint64_t renderTime = duration_cast<nanoseconds>(deltaTime).count();

            // If we rendered too fast set the thread to sleep for the rest of the time
            if (renderTime < deltaTimeNS)
            {
                uint64_t waitNanos = deltaTimeNS - renderTime;
                std::this_thread::sleep_for(std::chrono::nanoseconds(waitNanos));
            }
        }

        // Calculate delta time
        auto currentTime = Clock::now();
        auto deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Update delta time
        Time::delta = duration_cast<nanoseconds>(deltaTime).count();

        // Update global timer
        Time::globalTimer.update(Time::delta);

        // Count frames and update fps counter every second
        // replace through interval-system when the TimeManager has a "init" method
        frameCounter++;
        fpsTimer += Time::delta;
        if (fpsTimer > Time::SECOND)
        {
            Time::fps = frameCounter;
            fpsTimer -= Time::SECOND;
            frameCounter = 0;
        }

        // Update all callback-timer and call functions if necessary
        updateCallbackTimer(Time::delta);
    }


    void TimeManager::updateCallbackTimer(uint64_t delta)
    {
        auto& timers = Time::timerCallbacks;
        for (auto& timer : timers)
            timer.update(delta);

        // Check if one of the timers is done and remove it if so
        timers.erase(std::remove_if(timers.begin(), timers.end(),
                     [](const CallbackTimer& timer) { return timer.isFinished(); }), timers.end());
    }



}


