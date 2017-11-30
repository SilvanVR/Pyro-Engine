#include "transform.h"

namespace Pyro
{


    Transform::Transform()
        : Transform(Point3f(), Vec3f(1, 1, 1), Quatf())
    {}

    Transform::Transform(const Point3f& position)
        : Transform(position, Vec3f(1, 1, 1), Quatf())
    {}

    Transform::Transform(const Point3f& position, const Vec3f& scale)
        : Transform(position, scale, Quatf())
    {}

    Transform::Transform(const Point3f& position, const Quatf& rotation)
        : Transform(position, Vec3f(1, 1, 1), rotation)
    {}

    Transform::Transform(const Point3f& _position, const Vec3f& _scale, const Quatf& _rotation)
        : position(_position), scale(_scale), rotation(_rotation)
    {}

    //Return the transformation matrix from this Transform-Class
    Mat4f Transform::getTransformationMatrix() const
    {
        return Mat4f::trs(position, rotation, scale);
    }

    //Rotate the object so the forward vector points to the target
    void Transform::lookAt(const Point3f& target)
    {
        Mat4f lookAtMatrix = Mat4f::lookAt(position, target, Vec3f::up);
        rotation = lookAtMatrix.toQuaternion().conjugate();
    }

    void Transform::setXRotation(float angles)
    {
        Vec3f eulerAngles = rotation.toEulerAngles();
        rotation = Vec3f(angles, eulerAngles.y(), eulerAngles.z()).toQuaternion();
    }

    void Transform::setYRotation(float angles)
    {
        Vec3f eulerAngles = rotation.toEulerAngles();
        rotation = Vec3f(eulerAngles.x(), angles, eulerAngles.z()).toQuaternion();
    }

    void Transform::setZRotation(float angles)
    {
        Vec3f eulerAngles = rotation.toEulerAngles();
        rotation = Vec3f(eulerAngles.x(), eulerAngles.y(), angles).toQuaternion();
    }

}