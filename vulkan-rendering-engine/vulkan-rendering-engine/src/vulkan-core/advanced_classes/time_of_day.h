#ifndef TIME_OF_DAY
#define TIME_OF_DAY

#include "vulkan-core/scene_graph/nodes/node.h"
#include "sun/sun.h"

// Represents a time-of-day node which contains:
//  - a "24 hour" timer reduced to a custom amount of time
//     - this clock will be rendered as a gui-text on the bottom center
//  - several skyboxes (changes over time e.g. one for day, one for night)
//  - a sun node (which is responsible for rendering the sun)
//     - the sun node contains the directional light

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    // This time in seconds has to pass to pass 24 hours in simulation
    #define DEFAULT_24H_SIM_TIME            300
    #define DAY_IN_SECONDS                  86400
    #define HOUR_IN_SECONDS                 3600
    #define MINUTE_IN_SECONDS               60

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class GUI;
    class GUIText;
    class GUIImage;
    class Texture;
    class Cubemap;
    class Skybox;

    //---------------------------------------------------------------------------
    //  Structs
    //---------------------------------------------------------------------------

    struct DayTime
    {
        uint32_t hours, minutes, seconds;

        DayTime(uint32_t hour = 0, uint32_t min = 0, uint32_t sec = 0) : hours(hour), minutes(min), seconds(sec) {}

        // How much of the day has passed in %
        float getDayPercentage() const { return inSeconds() / (float)DAY_IN_SECONDS; }

        // return the day-time as seconds
        uint32_t inSeconds() const { return seconds + minutes * 60 + hours * 60 * 60; }

        // Return this daytime in the format hh:mm
        std::string toString(bool withSeconds) const;

        bool operator==(const DayTime& other) const { return hours == other.hours && minutes == other.minutes; }
        bool operator!=(const DayTime& other) const { return !(*this == other); }
        bool operator<(const DayTime& other) const;
        bool operator>(const DayTime& other) const{ return !(*this < other); }

    private:
        friend class TimeOfDay;
        void addSeconds(uint32_t seconds);
    };

    //---------------------------------------------------------------------------
    //  TimeOfDay class
    //---------------------------------------------------------------------------

    class TimeOfDay : public Node
    {

    public:
        // Create a TimeOfDay object which manages a simulation clock
        // @ _24hSimTime : 24h simulation corresponds to this amount of seconds in real-life
        TimeOfDay(RenderingEngine* renderer, uint32_t _24hSimTime = DEFAULT_24H_SIM_TIME, Sun* sun = nullptr);
        ~TimeOfDay();

        void update(float delta) override;

        // Add an callback to this system. It gets called everytime the time has been reached.
        // @ time: Valid values: 0:00 - 23:59
        // @ callJustOnce: Callback will be called only once
        CallbackID addCallback(const DayTime& time, const std::function<void()>& func, bool callJustOnce = false);
        void removeCallback(CallbackID callbackID);
        
        // Add an callback to this system. It gets called if the current time is between begin and end.
        // Depending on the current time the given value is 0.0f - 1.0f
        // @ begin: Begin of this event. (Valid values: 0:00 - 23:59)
        // @ end:   End of this event. (Valid values: 0:00 - 23:59)
        CallbackID addCallback(const DayTime& begin, const DayTime& end, const std::function<void(float)>& func);


        // Change the day-time (negative number means it remains as is)
        void            setDayTime(int hours = -1, int mins = -1, int seconds = -1);

        // Set the day-time with a percentage value (0.5f means 12:00:00);
        void            setDayTimeAsPercentage(float percentage);

        // Manage the simulation
        void            stopSimulation()    { bSimulate = false; }
        void            resumeSimulation()  { bSimulate = true; }
        void            toggleSimulation()  { bSimulate = !bSimulate; }

        // Set the simulation-time (the time which has to pass in seconds to simulate 24h)
        void            setSimulationTime(uint32_t newSimulationTime) { simulationTime = newSimulationTime; }
        void            setSimulationSpeed(float val){ simulationSpeed = val; }

        Sun*            getSun()            { assert(sun != nullptr); return sun; }
        Skybox*         getSkybox()         { assert(skybox != nullptr); return skybox; }
        const DayTime&  getDayTime() const  { return currentTime; }

    private:
        RenderingEngine* renderer;      // Pointer to the renderer to manipulate global states

        CubemapPtr  dayCubemap;         // Cubemap used for the day
        CubemapPtr  nightCubemap;       // Cubemap used for the night
        Skybox*     skybox;             // Pointer to the skybox
        Sun*        sun;                // Pointer to the sun
        DayTime     currentTime;        // Contains the time
        
        uint32_t    simulationTime;     // This time in seconds corresponds to 24h simulation
        bool        bSimulate;          // True: Simulation is running (clock advances)
        float       simulationSpeed;    // Makes the simulation faster or slower

        struct SimpleCallback
        {
            DayTime                 time;
            std::function<void()>   callback;
            CallbackID              id;
        };

        struct LerpCallback
        {
            DayTime                     begin;
            DayTime                     end;
            std::function<void(float)>  callback;
            CallbackID                  id;
        };

        std::vector<SimpleCallback> simpleCallbacks;
        std::vector<LerpCallback> lerpCallbacks;

        void advanceDayTime(float delta);
        void checkSimpleCallbacks();
        void checkLerpCallbacks();

        void setupDay();

#ifdef FREETYPE_LIB
        GUI*        gui;
        GUIText*    guiTimeText;
        GUIImage*   guiImageBackground;
        TexturePtr  guiImageBackgroundTexture;

    public:
        GUI*        getGUI() { assert(gui != nullptr); return gui; }
#endif
    };


}




#endif // !TIME_OF_DAY
