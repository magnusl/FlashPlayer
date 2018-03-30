#ifndef _CABCMETHODDEFINITION_H_
#define _CABCMETHODDEFINITION_H_

#include <stdint.h>
#include <vector>

namespace avm2
{

namespace abc
{

class CABCFile;

/**
 * Represents a method defined in the ABC file.
 */
class CAbcMethodDefinition
{
public:
    uint32_t    max_stack;
    uint32_t    local_count;
    uint32_t    init_scope_depth;
    uint32_t    max_scope_depth;
    CABCFile *  delivery;
    const std::vector<uint8_t> * byteCode;
};

} // namespace abc

} // namespace avm2
#endif