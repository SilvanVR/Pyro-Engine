#ifndef RESOURCE_OBJECT_H_
#define RESOURCE_OBJECT_H_

#include <string>

namespace Pyro
{

    class Scene;

    class ResourceObject
    {
    public:
        ResourceObject(const std::string& name = "") : m_name(name) {}
        virtual ~ResourceObject() {}

        // Getter's
        const std::string& getName() const { return m_name; }
        Scene* getBoundScene() const { return m_boundScene; }

        // Setter's
        void setName(const std::string& name) { m_name = name; }
        void setBoundScene(Scene* scene) { m_boundScene = scene; }

    protected:
        std::string     m_name;
        Scene*          m_boundScene;
    };

}


#endif // !RESOURCE_OBJECT_H_
