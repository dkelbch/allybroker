#include "platform.h"
#include "vmdebug.h"


const char *    gpsz_DebugLevel[E_VMDEBUG_MAX] =
{
    "none",
    "ERROR",
    "WARN",
    "INFO",
    "VERBOSE"
};

void        vmdebugPrint(const char* psz_Module, E_VMDEBUG e_Level, const char *fmt, ...)
{
    if( e_Level != E_VMDEBUG_OFF)
    {
        va_list ap;

        printf("[%s] %s ", psz_Module, gpsz_DebugLevel[e_Level]);
        va_start(ap, fmt);
        vprintf(fmt,ap);
        va_end(ap);
    }
}