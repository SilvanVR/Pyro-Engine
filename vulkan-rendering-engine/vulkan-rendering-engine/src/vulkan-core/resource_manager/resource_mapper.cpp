#include "resource_mapper.h"



namespace Pyro
{

    //---------------------------------------------------------------------------
    //  ResourceMapper - Public Methods
    //---------------------------------------------------------------------------

    bool ResourceMapper::add(ResourceID id, MappingValuePtr val)
    {
        if (val == nullptr)
            return false;

        bool nameAlreadyPresent = false;
        for (auto& idNamePair : m_IDMappings)
        {
            auto& values = idNamePair.second;
            nameAlreadyPresent = values.find(val) != values.end();
            if (nameAlreadyPresent)
                break;
        }
        m_IDMappings[id].insert(val);
        return nameAlreadyPresent;
    }

    ResourceID ResourceMapper::get(const MappingValuePtr val)
    {
        for (auto& idNamePair : m_IDMappings)
        {
            auto& values = idNamePair.second;
            for (auto it = values.begin(); it != values.end(); it++)
            {
                auto& value = *it;
                bool equal = value->equals(*val);
                if (equal)
                    return idNamePair.first;
            }
        }
        return RESOURCE_ID_INVALID;
    }

    void ResourceMapper::remove(ResourceID id)
    {
        m_IDMappings.erase(id);
    }


}