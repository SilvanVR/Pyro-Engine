#include "Random.h"


namespace math
{

    //---------------------------------------------------------------------------
    //  Static Member Initialization
    //---------------------------------------------------------------------------
    std::default_random_engine Random::engine{ std::random_device{}() };

}
