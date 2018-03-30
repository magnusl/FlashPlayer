#include "CABCFile.h"
#include <assert.h>
#include "CParseException.h"
#include <stdexcept>
#include <list>

using namespace avm2::io;
using namespace std;

namespace avm2
{
namespace abc
{

#define HAS_OPTIONAL        0x08
#define HAS_PARAM_NAMES     0x80

/*****************************************************************************/
/*                              Forward declarations                         */
/*****************************************************************************/
static bool verifyClassHierarcy(const vector<instance_info> & instances,
                                size_t classIndex,
                                list<size_t> & superClasses,
                                size_t & nativeClass);

static Object_MultiName abcMultiname2Multiname(size_t abcIndex,
                                        const CABCFile & abcFile,
                                        CStringTable & stringTable);

void Get_OptionDetail(CABCFile & abc,
    CBinarySource & _source, option_detail & detail)
{
    detail.val  = _source.GetU30();
    detail.kind = _source.GetU8();
}

void Get_OptionInfo(CABCFile & abc,
                    CBinarySource & _source, option_info & option)
{
    uint32_t count = _source.GetU30();
    for(size_t i = 0; i < count; ++i) {
        option_detail detail;
        Get_OptionDetail(abc, _source, detail);
        option.option.push_back(detail);
    }
}

void ParseMethodInfo(CABCFile & abc,
                     CBinarySource & _source, method_info & info)
{
    info.param_count    = _source.GetU30();
    info.return_type    = _source.GetU30();

    for(uint32_t i = 0; i < info.param_count; ++i) {
        info.param_type.push_back( _source.GetU30() );
    }
    
    info.name           = _source.GetU30();
    info.flags          = _source.GetU8();

    if (info.flags & HAS_OPTIONAL) {
        Get_OptionInfo(abc, _source, info.options);
    }
    if (info.flags & HAS_PARAM_NAMES) {
        for(size_t i = 0; i < info.param_count; ++i) {
            info.param_names.push_back(_source.GetU30());
        }
    }
}

void Get_Trait(CABCFile & abc,
               CBinarySource & _source, traits_info & trait)
{
    trait.name = abcMultiname2Multiname(_source.GetU30(), abc, 
        abc.GetStringTable()).qualifiedName;

    uint8_t kindattr = _source.GetU8();
    trait.kind = kindattr & 0x0f;
    trait.attr = kindattr >> 4;

    switch(trait.kind & 0x0f) {
    case Trait_Slot:
    case Trait_Const:
        trait.u.trait_slot.slotid       = _source.GetU30();
        trait.u.trait_slot.type_name    = _source.GetU30();
        trait.u.trait_slot.vindex       = _source.GetU30();
        trait.u.trait_slot.vkind        = trait.u.trait_slot.vindex ? _source.GetU8() : 0;
        break;
    case Trait_Class:
        trait.u.trait_class.slotid      = _source.GetU30();
        trait.u.trait_class.classi      = _source.GetU30();
        break;
    case Trait_Function:
        trait.u.trait_function.slotid   = _source.GetU30();
        trait.u.trait_function.function = _source.GetU30();
        break;
    case Trait_Getter:
    case Trait_Setter:
    case Trait_Method:
        trait.u.method.dispid           = _source.GetU30();
        trait.u.method.method           = _source.GetU30();
        break;
    }
    if ((trait.kind >> 4) & traits_info::ATTR_Metadata) {
        throw std::runtime_error("support for metadata not implemented.");
    }
}

void Get_InstanceInfo(CABCFile & abc,
    CBinarySource & _source, instance_info & instance)
{
    instance.name           = _source.GetU30();
    instance.super_name     = _source.GetU30();

    instance.flags          = _source.GetU8();
    if (instance.flags & 0x08 /* CONSTANT_ClassProtectedNs */) {
        instance.protectedNs = _source.GetU30();
    }
    uint32_t interfaceCount = _source.GetU30();
    for(size_t i = 0; i < interfaceCount; ++i) {
        instance.interfaces.push_back(_source.GetU30());
    }
    instance.iinit = _source.GetU30();
    size_t traitCount = _source.GetU30();
    instance.traits.resize(traitCount);
    for(size_t i = 0; i < traitCount; ++i) {
        Get_Trait(abc, _source, instance.traits[i]);
    }
}

void Get_ClassInfo(CABCFile & abc,
    CBinarySource & _source, class_info & info)
{
    info.cinit = _source.GetU30();
    uint32_t trait_count = _source.GetU30();
    info.traits.resize(trait_count);
    for(size_t i = 0; i < trait_count; ++i) {
        Get_Trait(abc, _source, info.traits[i]);
    }
}

void Get_ExceptionInfo(CABCFile & abc,
                       CBinarySource & _source,
                       exception_info & _info)
{
    _info.from      = _source.GetU30();
    _info.to        = _source.GetU30();
    _info.target    = _source.GetU30();
    _info.exc_type  = _source.GetU30();
    _info.var_name  = _source.GetU30();
}

void Get_MethodBodyInfo(CABCFile & abc,
                        CBinarySource & _source, 
                        method_body_info & _info)
{
    _info.method        = _source.GetU30();
    _info.max_stack = _source.GetU30();
    _info.local_count   = _source.GetU30();
    _info.init_scope_depth  = _source.GetU30();
    _info.max_scope_depth   = _source.GetU30();
        
    uint32_t code_length = _source.GetU30();
    _info.code.resize(code_length);
    if (!_source.Read(&_info.code[0], code_length)) {
        throw CParseException("Failed to read bytecode from method body.");
    }
    // Exceptions
    uint32_t exception_count = _source.GetU30();
    _info.exceptions.resize(exception_count);
    for(size_t i = 0; i < exception_count; ++i) {
        Get_ExceptionInfo(abc, _source, _info.exceptions[i]);
    }
    // traits
    size_t traitCount = _source.GetU30();
    _info.traits.resize(traitCount);
    for(size_t i = 0; i < traitCount; ++i) {
        Get_Trait(abc, _source, _info.traits[i]);
    }
}

CABCFile::CABCFile(CStringTable & strTable) : stringTable(strTable)
{
}

// Parses the ABC file from the source. Throws a CParseException if the
// parsing failed.
void CABCFile::Parse(CBinarySource & _source)
{
    // read the file version.
    minorVersion = _source.GetU16();
    majorVersion = _source.GetU16();

    constantPool.Parse(_source);
    ParseMethods(_source);
    ParseMetaData(_source);
    ParseClasses(_source);
    ParseScripts(_source);
    ParseMethodBodies(_source);
    ExtractMethods();
    ExtractClasses();
}

void CABCFile::ParseMethods(CBinarySource & _source)
{
    uint32_t count = _source.GetU30();
    if (count) {
        methodInfo.resize(count);
        for(size_t i = 0; i < count; ++i) {
            ParseMethodInfo(*this, _source, methodInfo[i]);
        }
    }
}

void CABCFile::ParseMetaData(CBinarySource & _source)
{
    uint32_t metadata_count = _source.GetU30();

    for(size_t i = 0; i < metadata_count; ++i)
    {
        uint32_t name       = _source.GetU30();
        uint32_t itemCount  = _source.GetU30();
        for(size_t item = 0; item < itemCount; ++item)
        {
            uint32_t key = _source.GetU30();
            if (key) {
                uint32_t value = _source.GetU30();
            }
        }
    }
}

void CABCFile::ParseClasses(CBinarySource & _source)
{
    uint32_t class_count = _source.GetU30();
    instanceInfo.resize(class_count);
    for(size_t i = 0; i < class_count; ++i) {
        Get_InstanceInfo(*this, _source, instanceInfo[i]);
    }
    classInfo.resize(class_count);
    for(size_t i = 0; i < class_count; ++i) {
        Get_ClassInfo(*this, _source, classInfo[i]);
    }
}

void CABCFile::ParseScripts(CBinarySource & _source)
{
    uint32_t count = _source.GetU30();
    for(uint32_t i = 0; i < count; ++i) {
        script_info info;
        info.init = _source.GetU30();
        uint32_t traitCount = _source.GetU30();
        info.traits.resize(traitCount);
        for(size_t i = 0; i < traitCount; ++i) {
            Get_Trait(*this, _source, info.traits[i]);
            scriptInfo.push_back(info);
        }
    }
}

void CABCFile::ParseMethodBodies(CBinarySource & _source)
{
    uint32_t count = _source.GetU30();
    for(uint32_t i = 0; i < count; ++i) {
        method_body_info body;
        Get_MethodBodyInfo(*this, _source, body);
        methodBodies.push_back(body);
    }
}

const CAbcMethodDefinition * CABCFile::GetInitScript() const
{
    if (scriptInfo.empty()) {
        return 0;
    }
    const script_info & initScript = scriptInfo.back();
    if (initScript.init >= methodInfo.size()) {
        return 0;
    }
    return &methodDefinitions[initScript.init];
}

Object_MultiName CABCFile::GetMultiName(size_t _abcIndex) const
{
    return abcMultiname2Multiname(_abcIndex, *this, stringTable);
}

const CAbcClassDefinition * CABCFile::GetClassDefinition(size_t _index) const
{
    if (_index >= classDefinitions.size()) {
        throw std::runtime_error("Invalid class index.");
    }
    return &classDefinitions[_index];
}

static Object_MultiName abcMultiname2Multiname(size_t abcIndex,
                                        const CABCFile & abcFile,
                                        CStringTable & stringTable)
{
    Object_MultiName mn;
    const CConstantPool & pool = abcFile.GetConstantPool();
    const abc::MultiName & abcName = pool.GetMultiName(abcIndex);

    switch(abcName.kind) {
    case CONSTANT_QName:
    case CONSTANT_QNameA:
        mn.Type = Object_MultiName::eQualifiedName;
        if (!abcName.u.QName.ns) {
            mn.qualifiedName.Namespace = 0;
        } else {
            const namespace_info & ns = pool.GetNamespace(abcName.u.QName.ns);
            if (ns.name == 0) {
                mn.qualifiedName.Namespace = 0;
            } else {
                mn.qualifiedName.Namespace = stringTable.set(pool.GetString(ns.name));
            }
        }
        mn.qualifiedName.Name = (abcName.u.QName.name == 0) ? 0 :
            stringTable.set(pool.GetString(abcName.u.QName.name));
        break;
    case CONSTANT_RTQName:
    case CONSTANT_RTQNameL:
        break;
    case CONSTANT_Multiname:
    case CONSTANT_MultinameA:
        {
            assert(false);
        }
    }
    return mn;
}

void CABCFile::ExtractMethods()
{
    if (classDefinitions.empty()) {
        classDefinitions.resize(instanceInfo.size());
    }

    methodDefinitions.resize(methodBodies.size());
    for(size_t i = 0, num = methodBodies.size();
        i < num;
        ++i)
    {
        const method_body_info & bInfo = methodBodies[i];
        const method_info & mInfo = methodInfo[i];

        methodDefinitions[i].local_count        = bInfo.local_count;
        methodDefinitions[i].max_scope_depth    = bInfo.max_scope_depth;
        methodDefinitions[i].max_stack          = bInfo.max_stack;
        methodDefinitions[i].byteCode           = &bInfo.code;
        methodDefinitions[i].delivery           = this;
    }
}

void CABCFile::ExtractClasses()
{
    if (classDefinitions.size() != instanceInfo.size()) {
        throw runtime_error("ExtractMethods() must be called before ExtractClasses.");
    }

    for(size_t i = 0, num = instanceInfo.size();
        i < num;
        ++i)
    {
        /////
        // Set the correct index.
        classDefinitions[i].abcIndex = i;

        /////
        // Verify the class hierarcy and determine it's superclasses.
        size_t nativeClass = 0;
        list<size_t> superClasses;
        if (!verifyClassHierarcy(instanceInfo, i, superClasses, nativeClass)) {
            throw std::runtime_error("Class validation failed.");
        }
        /////
        // Link this class to it's superclass.
        if (!superClasses.empty()) {
            classDefinitions[i].superClass = &classDefinitions[superClasses.front()];
        } else {
            classDefinitions[i].superClass = nullptr;
        }
        /////
        // Map it to a native class
        if (nativeClass) {
            Object_MultiName name = abcMultiname2Multiname(nativeClass, *this, stringTable);
            if (name.Type != Object_MultiName::eQualifiedName) {
                throw std::runtime_error("Name of super class must be a qualified name.");
            }
            classDefinitions[i].baseType = name.qualifiedName;
        } else {
            // what should we do here?
            assert(false);
        }
        /////
        // Now extract the fixed properties (traits)
        ExtractTraits(i, superClasses);
        
        /////
        // Set the static initializer
        if (classInfo[i].cinit >= methodDefinitions.size()) {
            throw std::runtime_error("Invalid cinit index.");
        }
        classDefinitions[i].staticInitializer = &methodDefinitions[classInfo[i].cinit];

        /////
        // Set the instance initialize
        if (instanceInfo[i].iinit >= methodDefinitions.size()) {
            throw std::runtime_error("Invalid iinit index.");
        }
        classDefinitions[i].instanceInitializer = &methodDefinitions[instanceInfo[i].iinit];
    }
}

inline uint32_t qnameToU32(const QName & name)
{
    return (((uint32_t) name.Name) << 16) | name.Namespace;
}


void CABCFile::ExtractTraits(size_t classInstance, 
                             const std::list<size_t> & superClasses)
{
    CAbcClassDefinition & classDef  = classDefinitions[classInstance];
    classDef.numSlots               = 0;

    std::vector<size_t> indicies;
    indicies.push_back(classInstance);  // start with the current class
    std::copy(superClasses.begin(), superClasses.end(), std::back_inserter(indicies));

    for(size_t i = 0; i < indicies.size(); ++i)
    {
        size_t classIndex = indicies[i];

        const std::vector<traits_info> & instanceTraits = instanceInfo[classIndex].traits;
        if (instanceTraits.empty()) {
            /////
            // This class does not define any traits, add a dummy entry for it so it's possible
            // to find the class in the hierarchy.
            QName dummy;
            dummy.Name = dummy.Namespace = 0;
            classDef.traits.push_back(TraitInfo());
            classDef.traitNames.push_back(dummy);
        } else {
            for(size_t i = 0, num = instanceTraits.size();
                i < num;
                ++i)
            {
                const traits_info & trait = instanceTraits[i];
                TraitInfo info;
                switch(trait.kind & 0x0f)   /**< want the type nibble */
                {
                case Trait_Slot:    /**< value */
                    info.class_index    = classIndex;
                    info.trait_type     = Trait_Slot;
                    info.index          = classDef.numSlots++;
                    break;
                case Trait_Method:
                    info.class_index    = classIndex;
                    info.trait_type     = Trait_Method;
                    info.index          = trait.u.method.method;
                    break;
                default:
                    // TODO: implement
                    assert(false);
                    break;
                }
                classDef.traits.push_back(info);
                classDef.traitNames.push_back(instanceTraits[i].name);
            }
        }
    }
}

static bool verifyClassHierarcy(const vector<instance_info> & instances,
                                size_t classIndex,
                                list<size_t> & superClasses,
                                size_t & nativeClass)
{
    const instance_info * currentInstance = &instances[classIndex];

    bool foundSuperClass;
    do {
        foundSuperClass = false;
        // iterate over the other classes
        for(size_t i = 0, num = instances.size();
            i < num;
            ++i)
        {
            if (currentInstance->super_name == instances[i].name) {
                /////
                // Found what seems to be the superclass.
                if (i == classIndex) {
                    // Loop detected, error
                    return false;
                } else if (find(superClasses.begin(), 
                    superClasses.end(), i) != superClasses.end()) {
                    // We have already visited this class.
                    return false;
                } else {
                    superClasses.push_back(i);
                    currentInstance = &instances[i];
                    foundSuperClass = true;
                }
            }
        }
    } while(foundSuperClass);

    /////
    // Didn't find this class in the ABC, so it must be a part of the runtime.
    nativeClass = currentInstance->name;
    return true;
}

} // namespace abc

} // namespace avm2
