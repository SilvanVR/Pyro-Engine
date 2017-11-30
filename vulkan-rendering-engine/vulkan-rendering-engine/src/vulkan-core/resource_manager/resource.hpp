#ifndef RESOURCE_H_
#define RESOURCE_H_

// Contains the Resource-Class and the IResourceSubManager-Class
// Resource-Class:
// Represents a handle to a resource loaded from disk (textures, models, shaders etc.)
// The underlying resource can be switched out by the resource-manager (e.g. if the file has changed on disk)
// The ResourceManager build those handles (and the underlying objects) and returns them if requested
// IResourceSubManager-Class
// Superclass for all different managers (model, shader, texture etc.)
// Manages the raw-data using a ResourceTable and has some convenience methods

#include "submanager/i_resource_submanager.hpp"
#include <type_traits>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Resource class
    //---------------------------------------------------------------------------

    template <class T, class T2 = T>
    class Resource
    {
    public:
        Resource(ResourceID id = RESOURCE_ID_INVALID, IResourceSubManager<T2>* resourceManager = nullptr);
        ~Resource();

        void                    makeGlobal()        { m_resourceManager->makeGlobal(m_resourceID); }
        ResourceID              getID() const       { return m_resourceID; }
        IResourceSubManager<T2>*   getResourceManager() const { return m_resourceManager; }

        const T*                get() const         { return indexTableSafeConst(); }
        T*                      get()               { return indexTableSafe(); }
        const T*                operator->() const  { return indexTableSafeConst(); }
        T*                      operator->()        { return indexTableSafe(); }

        bool isValid() const { return m_resourceID != RESOURCE_ID_INVALID && m_resourceManager != nullptr; }

        // Copy constructors
        Resource(std::nullptr_t null);
        Resource(const Resource<T, T2>& other);

        template <class T3, class = std::enable_if<std::is_convertible<T3*, T*>::value, void>::type>
        Resource(const Resource<T3, T2>& other);

        template <class T3>
        explicit Resource(const Resource<T3, T2>& other);

        // Assignment operators
        Resource<T>& operator=(std::nullptr_t null);
        Resource<T, T2>& operator=(const Resource<T, T2>& other);

        template <class T3, class = std::enable_if<std::is_convertible<T3*, T*>::value, void>::type>
        Resource<T>& operator=(const Resource<T3, T2>& other);

        bool operator==(const Resource<T>& other) const { return m_resourceID == other.m_resourceID && m_resourceManager == other.m_resourceManager; }
        bool operator!=(const Resource<T>& other) const { return !(*this == other); }
        bool operator==(std::nullptr_t null) const { return !isValid(); }
        bool operator!=(std::nullptr_t null) const { return isValid(); }

    private:
        ResourceID              m_resourceID;
        IResourceSubManager<T2>*   m_resourceManager;

        inline T* indexTable(){ return dynamic_cast<T*>((*m_resourceManager)[m_resourceID]); }
        inline T* indexTableSafe();

        inline T* indexTableConst() const { return dynamic_cast<T*>((*m_resourceManager)[m_resourceID]); }
        inline T* indexTableSafeConst() const;

        void addReference() {  if (isValid()) { m_resourceManager->addReference(m_resourceID); } }
        void deleteReference() { if (isValid()){ m_resourceManager->deleteReference(m_resourceID); } }
    };

    //---------------------------------------------------------------------------
    //  Resource class - Implementation
    //---------------------------------------------------------------------------

    template <class T, class T2>
    Resource<T,T2>::Resource(ResourceID id = RESOURCE_ID_INVALID, IResourceSubManager<T2>* resourceManager = nullptr)
        : m_resourceManager(resourceManager), m_resourceID(id)
    {
        addReference();
    }

    template <class T, class T2>
    Resource<T, T2>::~Resource()
    {
        deleteReference();
    }

    // Copy constructor + assignment operator for same type
    template <class T, class T2>
    Resource<T, T2>::Resource(const Resource<T, T2>& other)
    {
        m_resourceID = other.m_resourceID;
        m_resourceManager = other.m_resourceManager;
        addReference();
    }
    template <class T, class T2>
    Resource<T, T2>& Resource<T, T2>::operator=(const Resource<T, T2>& other)
    {
        deleteReference();
        m_resourceID = other.m_resourceID;
        m_resourceManager = other.m_resourceManager;
        addReference();
        return *this;
    }

    // const T* => T* const

    // Copy constructor + assignment operator for a different type, T3 must be subclass of T
    template <class T, class T2>
    template <class T3, class T4>
    Resource<T, T2>::Resource(const Resource<T3, T2>& other)
    {
        m_resourceID = other.getID();
        m_resourceManager = other.getResourceManager();
        addReference();
    }
    template <class T, class T2>
    template <class T3, class T4>
    Resource<T>& Resource<T, T2>::operator=(const Resource<T3, T2>& other)
    {
        deleteReference();
        m_resourceID = other.getID();
        m_resourceManager = other.getResourceManager();
        addReference();
        return *this;
    }

    // Explicit copy constructor
    template <class T, class T2>
    template <class T3>
    Resource<T, T2>::Resource(const Resource<T3, T2>& other)
    {
        m_resourceID = other.getID();
        m_resourceManager = other.getResourceManager();
        addReference();
    }

    // Allows assignment to nullptr
    template <class T, class T2>
    Resource<T, T2>::Resource(std::nullptr_t null)
    {
        m_resourceID = RESOURCE_ID_INVALID;
        m_resourceManager = nullptr;
    }
    template <class T, class T2>
    Resource<T>& Resource<T, T2>::operator=(std::nullptr_t null)
    {
        deleteReference();
        m_resourceID = RESOURCE_ID_INVALID;
        m_resourceManager = nullptr;
        return *this;
    }

    template <class T, class T2>
    inline T* Resource<T, T2>::indexTableSafe()
    {
        if (isValid())
            return indexTable();
        else
            Logger::Log("Tried to index an invalid resource-object with id #" + std::to_string(m_resourceID), LOGTYPE_WARNING);
        return nullptr;
    }

    template <class T, class T2>
    inline T* Resource<T, T2>::indexTableSafeConst() const
    {
        if (isValid())
            return indexTableConst();
        else
            Logger::Log("Tried to index an invalid resource-object with id #" + std::to_string(m_resourceID), LOGTYPE_WARNING);
        return nullptr;
    }

}



#endif //!RESOURCE_H_
