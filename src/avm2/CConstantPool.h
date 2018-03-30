#ifndef _CCONSTANTPOOL_H_
#define _CCONSTANTPOOL_H_

#include "binaryio.h"
#include "abctypes.h"
#include <vector>
#include <string>

namespace avm2
{

namespace abc
{

/**
 * ABC constant pool.
 */
class CConstantPool
{
public:
    // parses the constant pool from a binary source.
    void Parse(avm2::io::CBinarySource & _source);

    int32_t GetS32(size_t) const;
    uint32_t GetU32(size_t) const;
    double GetDouble(size_t) const;
    const MultiName & GetMultiName(size_t) const;
    const char * GetString(size_t) const;
    const namespace_info & GetNamespace(size_t) const;

private:
    std::vector<int32_t>        IntPool;
    std::vector<uint32_t>       UintPool;
    std::vector<double>         DoublePool;
    std::vector<std::string>    StringPool;
    std::vector<namespace_info> NamespacePool;
    std::vector<ns_set_info>    NamespaceSetPool;
    std::vector<MultiName>      MultiNamePool;
};

} // namespace abc

} // namespace avm2

#endif