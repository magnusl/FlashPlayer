#ifndef _CSTRINGTABLE_H_
#define _CSTRINGTABLE_H_

#include <stdint.h>
#include <vector>
#include <cstring>

namespace avm2
{

// String table for string lookup.
class CStringTable
{
public:
    typedef uint16_t index_t;
    /**
     * \brief   Adds a entry to the stringtable and returns the
     *          key to it.
     */
    index_t set(const char * a_String)
    {
        for(size_t i = 0; i < m_Strings.size(); ++i) {
            if (!strcmp(m_Strings[i].c_str(), a_String)) {
                return (index_t) i;
            }
        }
        m_Strings.push_back(std::string(a_String));
        return (index_t) (m_Strings.size() - 1);
    }

    index_t set(const std::string & a_String)
    {
        for(size_t i = 0; i < m_Strings.size(); ++i) {
            if (!strcmp(m_Strings[i].c_str(), a_String.c_str())) {
                return (index_t) i;
            }
        }
        m_Strings.push_back(std::string(a_String));
        return (index_t) (m_Strings.size() - 1);
    }

    /**
     * \brief   gets a string from the stringtable using the key.
     */
    const char * get(index_t a_Index)
    {
        if (a_Index >= m_Strings.size()) {
            return 0;
        }
        return  m_Strings[a_Index].c_str();
    }

protected:
    std::vector<std::string> m_Strings;
};

} // namespace avm2

#endif