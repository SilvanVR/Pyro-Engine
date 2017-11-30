#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "build_options.h"
#include "math/math_interface.h"

namespace Pyro
{


    //---------------------------------------------------------------------------
    //  Transform class
    //---------------------------------------------------------------------------

    class Transform
    {

    public:
        Point3f position;
        Vec3f   scale;
        Quatf   rotation;

        Transform();
        Transform(const Point3f& position);
        Transform(const Point3f& position, const Quatf& rotation);
        Transform(const Point3f& position, const Vec3f& scale);
        Transform(const Point3f& position, const Vec3f& scale, const Quatf& rotation);

        ~Transform() {};

        void setRotation(const Quatf& newRotation) { rotation = newRotation; }
        void setRotation(const Vec3f& direction) { rotation = Quatf::lookRotation(direction); }
        void setXRotation(float angles);
        void setYRotation(float angles);
        void setZRotation(float angles);
        void setPosition(const Point3f& pos) { position = pos; }
        void setScale(const Vec3f& newScale) { scale = newScale; }
        void setScale(float newScale) { scale = Vec3f(newScale, newScale, newScale); }

        //Return the transformation matrix from this Transform-Class
        Mat4f getTransformationMatrix() const;

        //Rotate the object so its look to the target
        void lookAt(const Point3f& target);

    };

}

#endif // !TRANSFORM_H_