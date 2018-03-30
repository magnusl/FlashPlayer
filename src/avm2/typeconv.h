#include "Handle.h"
#include "CStringTable.h"

namespace avm2
{
/**
 * \brief   Implements ECMA-262 ToPrimitive
 */
Handle_t ToPrimitive(Handle_t _value);

/**
 * \brief   Implements ECMA-262 ToBoolean
 */
Handle_t ToBoolean(Handle_t _value);

/**
 * \brief   Implements ECMA-262 ToString.
 */
std::string ToString(Handle_t a_Value, avm2::CStringTable & a_Table);

/**
 * \brief   Utility method for converting a string to a number.
 *
 * \param [in] a_String     The string to convert.
 * \param [out] a_Number    The converted number.
 */
bool ToNumber(const char * a_String, double & a_Number);

/**
 * \brief   Implements ECMA-262 ToNumber
 */
double ToNumber(Handle_t _value, avm2::CStringTable & _table);

/**
 * \brief   Implements ECMA-262 ToInt32
 */
int32_t ToInt32(Handle_t _value, avm2::CStringTable & _table);

/**
 * \brief   Implements "The Abstract Equality Comparison Algorithm" as defined in ECMA-262 Edition 3.
 */
bool Equality(Handle_t _x, Handle_t _y, CStringTable _stringTable);
    
/**
 * \brief   Implements the "The Strict Equality Comparison Algorithm" as defined in ECMA-262 Edition 3.
 */
bool StrictEquality(Handle_t _x, Handle_t _y, CStringTable & a_StringTable);

/** 
 * \brief The Abstract Relational Comparison Algorithm
 *
 * \retval  -1  undefined
 * \retval  0   false
 * \retval  1   true;
 */
int RelationalComparison(Handle_t _x, Handle_t _y, CStringTable & a_StringTable);

}