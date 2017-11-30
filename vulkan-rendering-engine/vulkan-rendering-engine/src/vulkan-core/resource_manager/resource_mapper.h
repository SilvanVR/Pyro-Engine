#ifndef RESOURCE_MAPPER_H_
#define RESOURCE_MAPPER_H_

#include "resource_table.hpp"
#include "data_types.hpp"

#include <unordered_set>
#include <string>
#include <memory>
#include <map>


namespace Pyro
{

    class MappingValue{
        std::string m_name;

    public:
        MappingValue(const std::string& str)
            : m_name(str) {}
        MappingValue(const char* str)
            : m_name(str) {}

        virtual std::string toString() const { return m_name; }
        bool equals(const MappingValue& other){ return toString() == other.toString(); }
    };

    class MappingFontValue : public MappingValue {
        uint32_t m_fontSize;

    public:
        MappingFontValue(const std::string& str, uint32_t fontSize)
            : MappingValue(str), m_fontSize(fontSize) {}

        std::string toString() const override { return MappingValue::toString() + "#" + std::to_string(m_fontSize); }
    };

    using MappingValuePtr = std::shared_ptr<MappingValue>;
    using MappingFontValuePtr = std::shared_ptr<MappingFontValue>;

    //---------------------------------------------------------------------------
    //  ResourceMapper class
    //---------------------------------------------------------------------------

    class ResourceMapper
    {
    public:
        ResourceMapper() {}

        // Add a new entry to the resourcemapper.
        // Returns false if an entry with this name was already present.
        bool add(ResourceID id, MappingValuePtr name);

        // Get a resourceid for a given name.
        // Return RESOURCE_ID_INVALID if not found.
        ResourceID get(MappingValuePtr val);

        // Deletes an entry for the given id.
        void remove(ResourceID id);

        // True if an id with the given name exists
        bool exists(MappingValuePtr val){ return get(val) != RESOURCE_ID_INVALID; }

    private:
        ResourceMapper(const ResourceMapper& other) = delete;
        ResourceMapper& operator=(const ResourceMapper& other) = delete;

        // Map a set of strings to a ResourceID
        std::map<ResourceID, std::unordered_set<MappingValuePtr>> m_IDMappings;
    };

}

#endif // !RESOURCE_MAPPER_H_

