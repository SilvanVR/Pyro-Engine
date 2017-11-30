#ifndef RAY_H_
#define RAY_H_

#include "build_options.h"

#include "math/math_interface.h"

namespace Pyro
{
    class Node;

    struct HitInfo
    {
        Point3f pos = Point3f();    // Hit-Position
        Node*   node = nullptr;     // Node which was hit

        HitInfo(const Point3f& _pos, Node* _node) : pos(_pos), node(_node) {}

        bool operator==(const HitInfo& hitInfo)
        {
            return this->pos == hitInfo.pos;
        }
    };

    //---------------------------------------------------------------------------
    //  Ray Class
    //---------------------------------------------------------------------------

    class Ray
    {

    public:
        static const float      DISTANCE_MAX;
        static const HitInfo    HIT_NOTHING;

        Ray(const Point3f& _position = Point3f(), const Vec3f& _direction = Vec3f(), float _distance = FLT_MAX)
            : origin(_position), direction(_direction), distance(_distance)
        {}

        const Point3f& getOrigin() const { return origin; }
        const Vec3f& getDirection() const { return direction; }
        float getDistance() const { return distance; }

        void setOrigin(const Point3f& pos) { this->origin = pos; }
        void setDirection(const Vec3f& dir) { this->direction = dir; }
        void setDistance(float distance){ this->distance = distance; }


    private:
        Point3f origin;
        Vec3f   direction;
        float   distance;

    };



}


#endif // !RAY_H_

