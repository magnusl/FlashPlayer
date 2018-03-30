#ifndef _CABCFILE_H_
#define _CABCFILE_H_

#include "CStringTable.h"
#include "CConstantPool.h"
#include "binaryio.h"
#include "CAbcClassDefinition.h"
#include "CAbcMethodDefinition.h"
#include "Names.h"
#include <list>
#include <map>

namespace avm2
{

namespace abc
{

/**
 * The ABC file is the delivery package for ABC bytecode.
 */
class CABCFile
{
public:
    explicit CABCFile(CStringTable &);

    // Parses the ABC file from the source. Throws a CParseException if the
    // parsing failed.
    void Parse(io::CBinarySource & _source);

    Object_MultiName GetMultiName(size_t _abcIndex) const;
    const CAbcClassDefinition * GetClassDefinition(size_t _index) const;

    const CConstantPool & GetConstantPool() const { return constantPool; }
    CStringTable & GetStringTable() { return stringTable; }
    const CAbcMethodDefinition * GetInitScript() const;

protected:
    CABCFile(const CABCFile &);
    CABCFile & operator=(const CABCFile &);

    void ParseMethods(io::CBinarySource & _source);
    void ParseMetaData(io::CBinarySource & _source);
    void ParseClasses(io::CBinarySource & _source);
    void ParseScripts(io::CBinarySource & _source);
    void ParseMethodBodies(io::CBinarySource & _source);
    void ExtractMethods();
    void ExtractClasses();
    void ExtractTraits(size_t, const std::list<size_t> &);

private:
    CConstantPool                       constantPool;
    CStringTable &                      stringTable;
    std::vector<CAbcMethodDefinition>   methodDefinitions;
    std::vector<CAbcClassDefinition>    classDefinitions;
    std::vector<method_info>            methodInfo;
    std::vector<method_body_info>       methodBodies;
    std::vector<class_info>             classInfo;
    std::vector<instance_info>          instanceInfo;
    std::vector<script_info>            scriptInfo;
    uint16_t                            minorVersion, majorVersion;
};

} // namespace abc

} // namespace avm2

#endif