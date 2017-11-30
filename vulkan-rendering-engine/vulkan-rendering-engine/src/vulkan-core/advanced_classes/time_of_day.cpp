#include "time_of_day.h"

#include "vulkan-core/resource_manager/resource_manager.h"
#include "../scene_graph/nodes/renderables/skybox.h"
#include "../data/material/texture/cubemap.h"
#include "../rendering_engine.h"
#include "../gui/gui_image.h"
#include "../gui/gui_text.h"
#include "../gui/gui.h"

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define MAX_AMBIENT                 1.0f
    #define MIN_AMBIENT                 0.05f
    #define MAX_SPECULAR                1.0f
    #define MIN_SPECULAR                0.0f
    #define SUNRISE_HOUR                6
    #define DAWN_HOUR                   20
    #define SUNRISE_FOG_DENSITY         0.01f
    #define DAWN_FOG_DENSITY            0.003f
    #define FOG_LOWER_LIMIT             -0.2f
    #define FOG_UPPER_LIMIT             0.8f
    #define SKYBOX_ROTATION_SPEED       0.005f

    //---------------------------------------------------------------------------
    //  Statics
    //---------------------------------------------------------------------------

    static IDGenerator<CallbackID> idGenerator;
    static CallbackID NULL_ID = idGenerator.generateID();

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    TimeOfDay::TimeOfDay(RenderingEngine* _renderer, uint32_t _24hSimTime, Sun* _sun)
        : Node("TimeOfDay"), renderer(_renderer), sun(_sun), 
          bSimulate(true), simulationTime(_24hSimTime), simulationSpeed(1.0f)
    {
        setDayTime(12,0,0);

        setupDay();

#ifdef FREETYPE_LIB
        guiTimeText = new GUIText("12:00:00", Vec2f(0,0));
        guiTimeText->setColor(Color::WHITE);
        guiTimeText->setAlign(TextAlign::CENTER);
        guiTimeText->setAnchor(Anchor::Center);
        guiTimeText->setLocalPos(Vec2f(0, guiTimeText->getHeight() * 0.5f));

        float textWidth = guiTimeText->getWidth();
        float textHeight = guiTimeText->getHeight();

        guiImageBackgroundTexture = TEXTURE("/textures/button4.dds");
        guiImageBackground = new GUIImage(guiImageBackgroundTexture, Rectf(textWidth * 1.2f, textHeight * 1.5f));
        guiImageBackground->setAnchor(Anchor::CenterBottom);
        guiImageBackground->setColor(Color(0.5f,0.5f,0.5f,0.9f));
        guiImageBackground->setLocalPos(Vec2f(-guiImageBackground->getWidth() * 0.5f, 
                                              -guiImageBackground->getHeight() - 5.0f));

        guiImageBackground->addChild(guiTimeText);

        gui = new GUI();
        gui->add({ guiImageBackground, guiTimeText });
#endif
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    TimeOfDay::~TimeOfDay()
    {
#ifdef FREETYPE_LIB
        delete guiImageBackground;
        delete guiTimeText;
        delete gui;
#endif
        delete skybox;
    }

    //---------------------------------------------------------------------------
    //  Update
    //---------------------------------------------------------------------------

    void TimeOfDay::update(float delta)
    {
        if (bSimulate)
            advanceDayTime(delta);

        checkSimpleCallbacks();
        checkLerpCallbacks();

        skybox->getTransform().rotation *= Quatf(Vec3f::up, SKYBOX_ROTATION_SPEED * delta);

    #ifdef FREETYPE_LIB
        guiTimeText->setText(currentTime.toString(true));
    #endif

        Node::update(delta);
    }

    //---------------------------------------------------------------------------
    //  Public Functions
    //---------------------------------------------------------------------------

    void TimeOfDay::setDayTime(int hours, int mins, int seconds)
    {
        if(hours >= 0)   currentTime.hours   = Mathf::clamp(hours, 0, 23);
        if(mins >= 0)    currentTime.minutes = Mathf::clamp(mins, 0, 59);
        if(seconds >= 0) currentTime.seconds = Mathf::clamp(seconds, 0, 59);
    }

    void TimeOfDay::setDayTimeAsPercentage(float percentage)
    {
        Mathf::clamp(percentage, 0.0f, 1.0f);
        uint32_t timeInSeconds = static_cast<uint32_t>(percentage * DAY_IN_SECONDS);

        currentTime.hours   = timeInSeconds / HOUR_IN_SECONDS;
        timeInSeconds -= currentTime.hours * HOUR_IN_SECONDS;

        // Handle special case where percentage = 1.0f
        currentTime.hours = currentTime.hours % 24;

        currentTime.minutes = timeInSeconds / MINUTE_IN_SECONDS;
        timeInSeconds -= currentTime.minutes * MINUTE_IN_SECONDS;
        currentTime.seconds = timeInSeconds;
    }

    CallbackID TimeOfDay::addCallback(const DayTime& time, const std::function<void()>& func, bool callJustOnce)
    {
        assert(time.hours < 24);

        SimpleCallback callback;
        callback.time       = time;
        callback.callback   = func;
        callback.id         = callJustOnce ? NULL_ID : idGenerator.generateID();

        simpleCallbacks.push_back(callback);

        return callback.id;
    }


    CallbackID TimeOfDay::addCallback(const DayTime& begin, const DayTime& end, const std::function<void(float)>& func)
    {
        assert(begin.hours < 24 && end.hours < 24 && begin != end);

        LerpCallback callback;
        callback.begin      = begin;
        callback.end        = end;
        callback.callback   = func;
        callback.id         = idGenerator.generateID();

        lerpCallbacks.push_back(callback);

        return callback.id;
    }

    void TimeOfDay::removeCallback(CallbackID callbackID)
    {
        simpleCallbacks.erase(std::remove_if(simpleCallbacks.begin(), simpleCallbacks.end(),[&](const SimpleCallback& e) {
            return e.id == callbackID;
        }), simpleCallbacks.end());
    }

    //---------------------------------------------------------------------------
    //  Private Functions
    //---------------------------------------------------------------------------

    void TimeOfDay::advanceDayTime(float delta)
    {
        float percentage = delta / simulationTime;
        uint32_t passedSeconds = static_cast<uint32_t>(percentage * (DAY_IN_SECONDS * simulationSpeed));

        currentTime.addSeconds(passedSeconds);
    }

    void TimeOfDay::checkSimpleCallbacks()
    {
        // Used to prevent calling callbacks several times per minute
        // This way callbacks will be called only once per new minute
        // The drawback of this is that if the time advances more than
        // a minute it will skip events during that time.
        static uint32_t lastMinute = 61;

        if (currentTime.minutes != lastMinute)
        {
            auto it = simpleCallbacks.begin();
            while (it != simpleCallbacks.end())
            {
                if (currentTime == it->time)
                {
                    it->callback();

                    // Now check if the current callback is an once callback and remove it if so
                    if(it->id == NULL_ID)
                        it = simpleCallbacks.erase(it);
                    else
                        it++;
                } else {
                    it++;
                }
            }
        }
        lastMinute = currentTime.minutes;
    }


    void TimeOfDay::checkLerpCallbacks()
    {
        for (auto& cb : lerpCallbacks)
        {
            if (currentTime > cb.begin && currentTime < cb.end)
            {
                // Calculate lerp-value and call the callback with the value
                uint32_t curTimeInSeconds   = currentTime.inSeconds();
                uint32_t beginTimeInSeconds = cb.begin.inSeconds();
                uint32_t endTimeInSeconds   = cb.end.inSeconds();

                float lerpVal = (curTimeInSeconds - beginTimeInSeconds) / (float)(endTimeInSeconds - beginTimeInSeconds);

                cb.callback(lerpVal);
            }
        }
    }


    void TimeOfDay::setupDay()
    {
        // --------------- Skybox ------------------
        dayCubemap   = CUBEMAP("/textures/cubemaps/tropical_sunny_day.dds");
        nightCubemap = CUBEMAP("/textures/cubemaps/full_moon.dds");
        skybox = new Skybox(dayCubemap, nightCubemap);

        addCallback({ DAWN_HOUR }, { 23, 59 }, [=](float lerp) {
            skybox->setBlendFactor(lerp);
        });
        addCallback({ SUNRISE_HOUR }, { 8 }, [=](float lerp) {
            skybox->setBlendFactor(1.0f - lerp);
        });


        // ----------------- Sun ---------------------
        if (sun)
        {
            // Sun Movement
            addCallback({ SUNRISE_HOUR }, { 12 }, [=](float lerp) {
                float x = Mathf::lerp(20.0f, -90.0f, lerp);
                sun->getTransform().setXRotation(x);
            });
            addCallback({ 12 }, { 23, 59 }, [=](float lerp) {
                float x = Mathf::lerp(-90.0f, -200.0f, lerp);
                sun->getTransform().setXRotation(x);
            });
            addCallback({ 0 }, { SUNRISE_HOUR }, [=](float lerp) {
                float x = Mathf::lerp(-200.0f, -340.0f, lerp);
                sun->getTransform().setXRotation(x);
            });

            // Sun Color
            addCallback({ SUNRISE_HOUR }, { 10 }, [=](float lerp) {
                sun->setColor(Mathf::lerp(Color::ORANGE, Color::WHITE, lerp));
            });
            addCallback({ DAWN_HOUR }, { 23, 0 }, [=](float lerp) {
                sun->setColor(Mathf::lerp(Color::WHITE, Color::ORANGE, lerp));
            });


            // Set the ambient + specular intensity based on the angle of the sun
            addCallback({ 0 }, { 23, 59 }, [=](float lerp) {
                float dot = Vec3f::down.dot(sun->getTransform().rotation.getForward());
                if(dot < 0) dot = 0;
                renderer->setAmbientIntensity(Mathf::lerp(MIN_AMBIENT, MAX_AMBIENT, dot));
                renderer->setEnvMapSpecularIntensity(Mathf::lerp(MIN_SPECULAR, MAX_SPECULAR, dot));
            });
        }

        // ----------------- FOG ---------------------
        if (SHADER_EXISTS("FOG"))
        {
            renderer->setFOGIsActive(true);
            renderer->setFOGDensity(0.0f);
            skybox->setFogDensity(0.0f);
            skybox->setFogLowerLimit(FOG_LOWER_LIMIT);
            skybox->setFogUpperLimit(FOG_UPPER_LIMIT);

            // ------- SUNRISE --------
            // Start to be foggy
            addCallback({ SUNRISE_HOUR }, { SUNRISE_HOUR + 2 }, [=](float lerp) {
                renderer->setFOGDensity(lerp * SUNRISE_FOG_DENSITY);
                skybox->setFogDensity(lerp);
                renderer->setFogColor(sun->getColor());
            });

            // End to be foggy
            addCallback({ SUNRISE_HOUR + 2 }, { SUNRISE_HOUR + 3 }, [=](float lerp) {
                float oneMinusLerp = 1.0f - lerp;
                renderer->setFOGDensity(oneMinusLerp * SUNRISE_FOG_DENSITY);
                skybox->setFogDensity(oneMinusLerp);
                renderer->setFogColor(sun->getColor());
            });

            // ------- DAWN --------
            // Start to be foggy
            addCallback({ DAWN_HOUR }, { 23 }, [=](float lerp) {
                renderer->setFOGDensity(lerp * DAWN_FOG_DENSITY);
                skybox->setFogDensity(lerp);
                renderer->setFogColor(sun->getColor());
            });

            // End to be foggy
            addCallback({ 22, 30 }, { 23, 0 }, [=](float lerp) {
                float oneMinusLerp = 1.0f - lerp;
                renderer->setFOGDensity(oneMinusLerp * DAWN_FOG_DENSITY);
                skybox->setFogDensity(oneMinusLerp);
                renderer->setFogColor(sun->getColor());
            });
        }
    }
    
    //---------------------------------------------------------------------------
    //  DayTime - Public Functions
    //---------------------------------------------------------------------------

    std::string DayTime::toString(bool withSeconds) const
    {
        std::string result = "";
        result += hours < 10 ? "0" + TS(hours) : TS(hours);
        result += ":";
        result += minutes < 10 ? "0" + TS(minutes) : TS(minutes);

        if (withSeconds)
        {
            result += ":";
            result += seconds < 10 ? "0" + TS(seconds) : TS(seconds);
        }

        return result;
    }

    void DayTime::addSeconds(uint32_t secs)
    {
        seconds += secs;

        while (seconds >= 60)
        {
            seconds -= 60;
            minutes++;
            while (minutes >= 60)
            {
                minutes -= 60;
                hours = (hours + 1) % 24;
            }
        }
    }

    bool DayTime::operator<(const DayTime& other) const
    {
        if(hours < other.hours)
            return true;
        else if(hours > other.hours)
            return false;
        else
        {
            if (minutes < other.minutes)
                return true;
            else if (minutes > other.minutes)
                return false;
            else
            {
                if (seconds < other.seconds)
                    return true;
                else if (seconds > other.seconds)
                    return false;
            }
        }
        return true;
    }

}