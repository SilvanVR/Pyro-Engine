#include "utils.h"

#include <fstream>
#include <sstream>

namespace Pyro
{

    std::vector<std::string> splitString(const std::string& string, char delim)
    {
        std::string str = string;
        for (unsigned int i = 0; i < str.length(); i++)
        {
            if (str[i] == delim)
                str[i] = ' ';
        }

        std::vector<std::string> array;
        std::stringstream ss(str);
        std::string temp;
        while (ss >> temp)
            array.push_back(temp);

        return array;
    }

    std::string stringToLower(const std::string& str)
    {
        std::string s = str;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

}