/*
*  Random - Class header file.
*  Using the C++ 11 <random> header file.
*  Using the standard engine. (For more information see https://isocpp.org/files/papers/n3551.pdf) //possible better: std::mt19937
*  Static member declarations in Random.cpp
*
*  Date:       26.03.2016
*  Creator:    Silvan Hau
*/

#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <random>
#include "Vector3.h"
#include "Point.h"
#include "Quaternion.h"

namespace math
{
    //---------------------------------------------------------------------------
    //  Random Class
    //---------------------------------------------------------------------------
    class Random
    {
    private:
        static std::default_random_engine engine; 

    public:
        static                       int           randomInt        (int min, int max);         //Returns an random integer between [min,max]. Both bounds are included.
        static                       int           randomInt        (int max);                  //Returns an random integer between [0,max]. Both bounds are included.
        static                       float         randomFloat      (float min, float max);     //Returns an random float between [min,max]. Both bounds are included.
        static                       float         randomFloat      (float max);                //Returns an random float between [0,max]. Both bounds are included.
        static                       double        randomDouble     (double min, double max);   //Returns an random double between [min,max]. Both bounds are included.
        static                       double        randomDouble     (double max);               //Returns an random double between [0,max]. Both bounds are included.
        template <typename T> static Vec3<T>       randomVec3       (T min, T max);             //Returns an random Point (3D) with all components between [min,max]. Specialization with <> e.g. randomVec3<float>(0,1).
        template <typename T> static Point<T>      randomPoint      (T min, T max);             //Returns an random Point (3D) with all components between [min,max]. Specialization with <> e.g. randomVec3<float>(0,1).
        template <typename T> static Quaternion<T> randomQuaternion ();                         //Returns an random (normalized) quaternion.
        template <typename T> static T             value            ();                         //Returns a value between [0,1]. Both bounds are included. Specialization with <> e.g. Random::value<float>().
    };


    //---------------------------------------------------------------------------
    //  Static functions
    //---------------------------------------------------------------------------

    //Returns an random integer between [min,max]. Both bounds are included.
    inline int Random::randomInt(int min, int max)
    {
        return std::uniform_int_distribution<int>{min, max}(engine);
    }
    //Returns an random integer between [0,max]. Both bounds are included.
    inline int Random::randomInt(int max = 1)
    {
        return randomInt(0, max);
    }

    //Returns an random float between [min,max]. Both bounds are included.
    inline float Random::randomFloat(float min, float max)
    {
        return std::uniform_real_distribution<float>{min, max}(engine);
    }
    //Returns an random float between [0,max]. Both bounds are included.
    inline float Random::randomFloat(float max = 1)
    {
        return randomFloat(0, max);
    }

    //Returns an random double between [min,max]. Both bounds are included.
    inline double Random::randomDouble(double min, double max)
    {
        return std::uniform_real_distribution<double>{min, max}(engine);
    }
    //Returns an random double between [0,max]. Both bounds are included.
    inline double Random::randomDouble(double max = 1)
    {
        return randomDouble(0, max);
    }

    //Returns an random Vector3 with all components between [min,max]. Specialization with <> e.g. randomVec3<float>(0,1).
    template <typename T> inline
    Vec3<T> Random::randomVec3(T min, T max)
    {
        auto distribution = std::uniform_real_distribution<T>{ min, max };
        return Vec3<T>(distribution(engine), distribution(engine), distribution(engine));
    }
    //Returns an random Vector3 with all components between [min,max]. <int> specialization.
    template <> inline
    Vec3<int> Random::randomVec3(int min, int max)
    {
        auto distribution = std::uniform_int_distribution<int>{ min, max };
        return Vec3<int>(distribution(engine), distribution(engine), distribution(engine));
    }

    //Returns an random Vector3 with all components between [min,max]. Specialization with <> e.g. randomVec3<float>(0,1).
    template <typename T> inline
    Point<T> Random::randomPoint(T min, T max)
    {
        auto distribution = std::uniform_real_distribution<T>{ min, max };
        return Point<T>(distribution(engine), distribution(engine), distribution(engine));
    }
    //Returns an random Vector3 with all components between [min,max]. <int> specialization.
    template <> inline
    Point<int> Random::randomPoint(int min, int max)
    {
        auto distribution = std::uniform_int_distribution<int>{ min, max };
        return Point<int>(distribution(engine), distribution(engine), distribution(engine));
    }

    //Returns an random (normalized) quaternion.
    template <typename T> inline
    Quaternion<T> Random::randomQuaternion()
    {
        auto distribution = std::uniform_real_distribution<T>{ 0, 1 };
        return Quaternion<T>(distribution(engine), distribution(engine), distribution(engine), distribution(engine)).normalize();
    }

    //Returns a value between [0,1]. Both bounds are included. Specialization with <> e.g. Random::value<float>().
    template <typename T> inline
    T Random::value()
    {
        return std::uniform_real_distribution<T>{ 0, 1 }(engine);
    }
    //Returns a value between [0,1]. Both bounds are included. <int> specialization.
    template <> inline
    int Random::value()
    {
        return std::uniform_int_distribution<int>{ 0, 1 }(engine);
    }
}
#endif