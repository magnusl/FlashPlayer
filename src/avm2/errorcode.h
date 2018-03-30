#ifndef _ERRORCODE_H_
#define _ERRORCODE_H_

namespace avm2
{
    typedef enum {
        Err_Ok,
        Err_InvalidParameter,
        Err_NotImplemented,
        Err_InternalError,
        Err_VerfificationError
    } ErrorCode_t;
}

#endif