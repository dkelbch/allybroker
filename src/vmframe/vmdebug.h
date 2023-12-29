/*!
    debug interface to formalize the
    debugging and logging function
*/

#ifndef        VM_DEBUG_H
#define         VM_DEBUG_H
typedef enum
{
    E_VMDEBUG_OFF,
    E_VMDEBUG_ERR,
    E_VMDEBUG_WARN,
    E_VMDEBUG_INFO,
    E_VMDEBUG_VERBOSE,
    E_VMDEBUG_MAX
} E_VMDEBUG;

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

void        vmdebugPrint(const char* psz_Module, E_VMDEBUG e_Level, const char *fmt, ...);

#define         VM_PRINT_DBG(x)    vmdebugPrint x
#define         VM_PRINT_LOG(x) vmdebugPrint x

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // VM_DEBUG_H