#ifndef OBJECT_H_
#define OBJECT_H_

#include <stdint.h>
#include <string>

namespace Pyro
{

    using ObjectID = uint16_t;

    //---------------------------------------------------------------------------
    //  Forward Declarations
    //---------------------------------------------------------------------------

    class Scene;

    //---------------------------------------------------------------------------
    //  Object - Class
    //---------------------------------------------------------------------------

    // Local-Object: Object is bound to the current scene and will be deleted upon scene deconstruction
    // Global-Object: Object is global and it will not be destroyed when the scene changes
    class Object
    {
    public:
        enum EType
        {
            LOCAL,
            GLOBAL
        };

        Object(EType type = LOCAL, const std::string& name = "", Scene* scene = nullptr);
        virtual ~Object();

        ObjectID getID() const { return id; } 
        const std::string& getName() const { return name; }
        void setName(const std::string& name) { this->name = name; }
        const Scene* getBoundScene(){ return myScene; }

        // Make the object a persistent object -> wont be deleted on scene deconstruction.
        void makeGlobal();

        // Check whether this object is a global-object and will not be destroyed when the cur-scene gets destroyed
        bool isGlobal(){ return type == GLOBAL; }

        // Check whether this object is local (or bound to a scene)
        bool isLocal(){ return type== LOCAL; }

        void changeType(EType newType);

    protected:
        Scene*          myScene;

    private:
        EType           type;
        ObjectID        id;
        std::string     name;
    };


}

#endif // !OBJECT_H_
