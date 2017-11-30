#include "ray.h"


namespace Pyro
{
    const float     Ray::DISTANCE_MAX = FLT_MAX;
    const HitInfo   Ray::HIT_NOTHING{ Point3f(FLT_MAX, FLT_MAX), nullptr };


}
