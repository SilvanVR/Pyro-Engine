#ifndef UTILS_H_
#define UTILS_H_

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace Pyro
{

    std::vector<std::string> splitString(const std::string& string, char delim);
    std::string stringToLower(const std::string& str);

    template <typename T>
    std::string toStringWithPrecision(const T a_value, const int n = 6)
    {
        std::ostringstream out;
        out << std::setprecision(n) << a_value;
        return out.str();
    }

    // Helper function to remove a object from a std::vector
    template <typename T>
    bool removeObjectFromList(std::vector<T>& objects, const T& object)
    {
        //objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
        for (uint32_t i = 0; i < objects.size(); i++)
        {
            if (objects[i] == object)
            {
                objects.erase(objects.begin() + i);
                return true;
            }
        }
        return false;
    }

    // Helper Class which generates UNIQUE ID's. They will be reserved unless you free them manually with freeID()
    // It generates depending on the data-type id's before it crashes e.g. for unsigned char's 255.
    // Minimum ID is 1 because 0 is reserved for an INVALID id.
    // Be careful, should be used with UNSIGNED TYPES otherwise you loose half of the potential ID's.
    template <typename T, T max = std::numeric_limits<T>::max()>
    class IDGenerator
    {
        enum
        {
            FREE = false,
            USED = true
        };

        // Array which stores the information which ID is still free
        bool m_IDArray[max] = {};
        T m_AmountOfUsedIDs = 0;

    public:
        // Return the maximum amount of possible ID's
        T maxIDs() { return max; }

        // Generate a unique id and return it
        T generateID()
        {
            T id = 0;
            for (; id < max; id++)
            {
                if (m_IDArray[id] == FREE) {
                    m_IDArray[id] = USED;
                    m_AmountOfUsedIDs++;
                    break;
                }
            }
            if (id == max)
                Logger::Log("Exceeded ID limit in IDGenerator. #Min: 1 "
                    "#Max: " + std::to_string(max), LOGTYPE_ERROR);
            return id + 1;
        }

        // Free the given id
        void freeID(T id)
        {
            T realID = id - 1;
            if (m_IDArray[realID] == USED)
            {
                m_IDArray[realID] = FREE;
                m_AmountOfUsedIDs--;
            }
        }

        T getAmountOfFreeIDs()
        {
            return maxIDs() - m_AmountOfUsedIDs;
        }

        T getAmountOfUsedIDs()
        {
            return m_AmountOfUsedIDs;
        }

    };
}


#endif // !UTILS_H_
