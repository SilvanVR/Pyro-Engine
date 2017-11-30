#include "object.h"

#include "scene_manager.h"
#include "utils/utils.h"

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Static Declarations
    //---------------------------------------------------------------------------

    static IDGenerator<ObjectID> iDGenerator;

    //---------------------------------------------------------------------------
    //  Constructor
    //---------------------------------------------------------------------------

    Object::Object(EType _type, const std::string& _name, Scene* scene)
        : type(_type), name(_name), myScene(scene)
    {
        id = iDGenerator.generateID();

        if(myScene == nullptr && isLocal())
            myScene = SceneManager::getCurrentScene();

        if(myScene) myScene->addObject(this);
    }

    //---------------------------------------------------------------------------
    //  Destructor
    //---------------------------------------------------------------------------

    Object::~Object()
    {
        iDGenerator.freeID(id);
        if(myScene) myScene->removeObject(this);
    }

    //---------------------------------------------------------------------------
    //  Public Methods
    //---------------------------------------------------------------------------

    void Object::makeGlobal()
    {
        changeType(Object::GLOBAL);
    }

    //---------------------------------------------------------------------------
    //  Protected Methods
    //---------------------------------------------------------------------------

    void Object::changeType(EType newType)
    {
        type = newType;

        if(myScene) myScene->removeObject(this);

        if(isLocal())
            myScene = SceneManager::getCurrentScene();
        else
            myScene = nullptr;

        if (myScene) myScene->addObject(this);
    }


}