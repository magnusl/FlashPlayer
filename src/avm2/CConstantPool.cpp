#include "CConstantPool.h"
#include "CParseException.h"

using namespace avm2::io;

namespace avm2
{

namespace abc
{

int32_t CConstantPool::GetS32(size_t index) const
{
    if (index >= IntPool.size()) {
        throw std::runtime_error("Invalid index into integer constant pool.");
    }
    return IntPool[index];
}

uint32_t CConstantPool::GetU32(size_t index) const
{
    if (index >= UintPool.size()) {
        throw std::runtime_error("Invalid index into unsigned integer constant pool.");
    }
    return UintPool[index];
}

double CConstantPool::GetDouble(size_t index) const
{
    if (index >= DoublePool.size()) {
        throw std::runtime_error("Invalid index into double constant pool.");
    }
    return DoublePool[index];
}

const MultiName & CConstantPool::GetMultiName(size_t index) const
{
    if (index >= MultiNamePool.size()) {
        throw std::runtime_error("Invalid index into multiname constant pool.");
    }
    return MultiNamePool[index];
}

const char * CConstantPool::GetString(size_t index) const
{
    if (index >= StringPool.size()) {
        throw std::runtime_error("Invalid index into string constant pool.");
    }
    return StringPool[index].c_str();
}

const namespace_info & CConstantPool::GetNamespace(size_t index) const
{
    if (index >= NamespacePool.size()) {
        throw std::runtime_error("Invalid index into namespace pool.");
    }
    return NamespacePool[index];
}

static namespace_info Get_NamespaceInfo(CBinarySource & _source)
{
    namespace_info info;
    info.kind = _source.GetU8();
    info.name = _source.GetU30();
    return info;
}

static void Get_NamespaceSet(CBinarySource & _source, ns_set_info & ns)
{
    uint32_t count = _source.GetU30();
    for(uint32_t i = 0; i < count; ++i) {
        ns.ns.push_back(_source.GetU30());
    }
}

static void Get_Multiname(CBinarySource & _source, MultiName & mn)
{
    mn.kind = _source.GetU8();
    switch(mn.kind) {
    case CONSTANT_QName:
    case CONSTANT_QNameA:
        mn.u.QName.ns   = _source.GetU30();
        mn.u.QName.name = _source.GetU30(); 
        break;
    case CONSTANT_RTQName:
    case CONSTANT_RTQNameA:
        mn.u.RTQName.name = _source.GetU30();
        break;
    case CONSTANT_RTQNameL:
    case CONSTANT_RTQNameLA:
        break;
    case CONSTANT_Multiname:
    case CONSTANT_MultinameA:
        mn.u.Multiname.name     = _source.GetU30();
        mn.u.Multiname.ns_set   = _source.GetU30();
        break;
    case CONSTANT_MultinameL:
    case CONSTANT_MultinameLA:
        mn.u.MultinameL.ns_set = _source.GetU30();
        break;
    default:
        throw CParseException("Unknown multiname kind.");
    }
}

std::string Get_StringInfo(CBinarySource & io)
{
    uint32_t len = io.GetU30();
    if (!len) {
        return "";
    }
    std::vector<char> str(len + 1); 
    if (!io.Read(&str[0], len)) {
        throw std::runtime_error("failed to read utf8 encoded string.");
    }
    str[len] = 0;
    return std::string(&str[0]);
}

// parses the constant pool from a binary source.
void CConstantPool::Parse(CBinarySource & _source)
{
    // signed integers.
    uint32_t count = _source.GetU30();
    IntPool.push_back(0);
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            IntPool.push_back( _source.GetS32() );
        }
    }
    // unsigned integers
    count = _source.GetU30();
    UintPool.push_back(0);
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            UintPool.push_back( _source.GetU32() );
        }
    }
    // doubles
    count = _source.GetU30();
    DoublePool.push_back(0);    // first should be NaN.
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            DoublePool.push_back( _source.GetDouble() );
        }
    }
    // strings
    count = _source.GetU30();
    StringPool.push_back("*");      // first entry is always empty.
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            StringPool.push_back( Get_StringInfo(_source) );
        }
    }
    // namespace
    count = _source.GetU30();
    NamespacePool.push_back(namespace_info());
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            namespace_info info = Get_NamespaceInfo(_source);
            NamespacePool.push_back( info );
        }
    }
    // namespace set
    count = _source.GetU30();
    NamespaceSetPool.push_back(ns_set_info());
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            ns_set_info info;
            Get_NamespaceSet(_source, info);
            NamespaceSetPool.push_back(info);
        }
    }
    // multiname
    count = _source.GetU30();
    MultiNamePool.push_back(MultiName());
    if (count > 1) {
        for(uint32_t i = 0; i < (count-1); ++i) {
            MultiName mn;
            Get_Multiname(_source, mn);
            MultiNamePool.push_back(mn);
        }
    }
}

} // namespace abc

} // namespace avm2