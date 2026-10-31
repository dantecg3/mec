#ifndef MEC_ERROR_H
#define MEC_ERROR_H

#include "str.h"

typedef uint8 ERROR_Code;
#define ERROR_CODE_NONE (ERROR_Code)0

typedef struct {
    ERROR_Code code;
    STR_View msg;
} ERROR;

static inline ERROR error(ERROR_Code code, const char *literal)
{
    return (ERROR) {
        .code = code,
        .msg = str_view(literal)
    };
}

static inline ERROR error_null()
{
    return (ERROR) {
        .code = ERROR_CODE_NONE,
        .msg = {0}
    };
}

static inline bool error_is_null(ERROR error)
{
    return error.code == ERROR_CODE_NONE;
}

#endif // MEC_ERROR_H
