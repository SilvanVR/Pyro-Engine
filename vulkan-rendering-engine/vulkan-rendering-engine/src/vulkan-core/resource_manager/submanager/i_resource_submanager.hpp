#ifndef I_RESOURCE_MANAGER_H_
#define I_RESOURCE_MANAGER_H_

#include "../resource_table.hpp"
#include "../resource_mapper.h"
#include "data_types.hpp"
#include <map>

// Interface for an arbitrary resourcemanager class.

namespace Pyro
{
    //---------------------------------------------------------------------------
    //  IResourceSubManager class
    //---------------------------------------------------------------------------

    template <typename T>
    class IResourceSubManager
    {
    public:
        virtual ~IResourceSubManager() {}

        virtual void init() = 0;
        virtual void destroy() { m_globalResources.clear(); };

        void addReference(ResourceID id) { m_resourceTable.incrementReference(id); }
        void deleteReference(ResourceID id);
        void makeGlobal(ResourceID id);
        virtual void updateResources() {}

        uint32_t getAmountOfResources() { return m_resourceTable.getAmountOfResources(); }

        T* operator[](ResourceID i) { return m_resourceTable[i]; }
        const T* operator[](ResourceID i) const { return m_resourceTable[i]; }

    protected:
        // Manages the raw data
        ResourceTable<T> m_resourceTable;
        ResourceMapper m_globalResourceMapper;  // Maps names to IDs, so a resource can have several names

        void addGlobalResource(Resource<T, T> res, MappingValuePtr val = nullptr);
        bool isValid(ResourceID id) { return m_resourceTable[id] != nullptr; }

        virtual ResourceID addToResourceTable(T* resource) = 0;
        virtual void removeFromSceneMapper(ResourceID id) {}

    private:
        std::vector<Resource<T, T>> m_globalResources;  // Stores handles to global resources
    };

    template <class T>
    void IResourceSubManager<T>::deleteReference(ResourceID id) 
    {
        bool wasDeleted = m_resourceTable.decrementReference(id);
        if (wasDeleted)
        {
            m_globalResourceMapper.remove(id);
            removeFromSceneMapper(id);
        }
    }

    template <class T>
    void IResourceSubManager<T>::addGlobalResource(Resource<T, T> res, MappingValuePtr val)
    { 
        m_globalResources.push_back(res);
        bool wasAlreadyPresent = m_globalResourceMapper.add(res.getID(), val);
        if (wasAlreadyPresent)
            Logger::Log("Global resource '" + val->toString() + "' was already present. "
                        "Trying to get the resource won't work. Consider using a different name.", LOGTYPE_WARNING);
    }

    template <class T>
    void IResourceSubManager<T>::makeGlobal(ResourceID id)
    { 
        Resource<T> res(id, this);
        addGlobalResource(res, res->getName());
    }


}


#endif // !I_RESOURCE_MANAGER_H_
