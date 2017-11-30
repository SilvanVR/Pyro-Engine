#ifndef RESOURCE_TABLE_H_
#define RESOURCE_TABLE_H_

#include "resource_object.hpp"
#include "utils/utils.h"
#include <functional>
#include <assert.h>
#include <limits>
#include <vector>

namespace Pyro
{

    //---------------------------------------------------------------------------
    //  Defines
    //---------------------------------------------------------------------------

    #define RESOURCE_ID_INVALID 0

    //---------------------------------------------------------------------------
    //  Type Declarations
    //---------------------------------------------------------------------------

    using ResourceID = unsigned short;
    
    //---------------------------------------------------------------------------
    //  ResourceTable class
    //---------------------------------------------------------------------------

    template <typename T>
    class ResourceTable
    {
        static const ResourceID DEFAULT_SIZE = std::numeric_limits<ResourceID>::max();

        struct SharedData
        {
            ResourceObject* ptr;
            int referenceCount;
            SharedData(ResourceObject* p) : ptr(p), referenceCount(0) {}
            ~SharedData(){ delete ptr; }
        };

    public:
        ResourceTable() {}

        uint32_t getAmountOfResources() { return static_cast<uint32_t>(idGenerator.getAmountOfUsedIDs()); }
        ResourceID maxPossibleResources() { return idGenerator.maxIDs(); }

        T* operator[](ResourceID i) { return m_resourceTable[i] == nullptr ? nullptr : dynamic_cast<T*>(m_resourceTable[i]->ptr); }
        const T* operator[](ResourceID i) const { return m_resourceTable[i] == nullptr ? nullptr : dynamic_cast<T*>(m_resourceTable[i]->ptr); }

        template <typename T>
        ResourceID add(T* pData)
        {
            ResourceID nextFreeID = idGenerator.generateID();
            m_resourceTable[nextFreeID] = new SharedData(pData);
            return nextFreeID;
        }

        void incrementReference(ResourceID id)
        {
            assert(m_resourceTable[id] != nullptr);
            m_resourceTable[id]->referenceCount++;
        }

        bool decrementReference(ResourceID id)
        {
            assert(m_resourceTable[id] != nullptr);
            m_resourceTable[id]->referenceCount--;
            if (m_resourceTable[id]->referenceCount == 0)
            {
                //Logger::Log("Deleting resource '" + m_resourceTable[id]->ptr->getName() + "'...", LOGTYPE_INFO, LOG_LEVEL_NOT_SO_IMPORTANT);
                delete m_resourceTable[id];
                m_resourceTable[id] = nullptr;
                idGenerator.freeID(id);
                return true;
            }
            return false;
        }

        // Used for hot-reloading. Deletes the old data at index "id" and puts the new data in their.
        void exchangeData(ResourceID id, ResourceObject* newData)
        {
            assert(m_resourceTable[id] != nullptr);
            delete m_resourceTable[id]->ptr;
            m_resourceTable[id]->ptr = newData;
        }

        // Find the first resource where "func" is true
        ResourceID find(const std::function<bool(T* ptr)>& func)
        {
            int visitedAmount = 0;
            for (int i = 0; i < idGenerator.maxIDs(); i++)
            {
                if (m_resourceTable[i] != nullptr)
                {
                    if (func(dynamic_cast<T*>(m_resourceTable[i]->ptr)))
                        return i;

                    visitedAmount++;
                    if (visitedAmount == getAmountOfResources())
                        break;
                }
            }
            return RESOURCE_ID_INVALID;
        }

        // Find all resources where "func" is true
        std::vector<ResourceID> findAll(const std::function<bool(T* ptr)>& func)
        {
            std::vector<ResourceID> ids;
            int visitedAmount = 0;
            for (int i = 0; i < idGenerator.maxIDs(); i++)
            {
                if (m_resourceTable[i] != nullptr)
                {
                    if (func(dynamic_cast<T*>(m_resourceTable[i]->ptr)))
                        ids.push_back(i);

                    visitedAmount++;
                    if (visitedAmount == getAmountOfResources())
                        break;
                }
            }
            return ids;
        }

    private:
        SharedData* m_resourceTable[DEFAULT_SIZE];
        IDGenerator<ResourceID, DEFAULT_SIZE> idGenerator;
    };

}


#endif