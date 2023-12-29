#ifndef    PLATFORM_H
#define     PLATFORM_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "linklist.h"

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;
typedef char *              PCHAR;
typedef void *              PVOID;
typedef int                 HANDLE;

typedef struct      _vmstring
{
    PCHAR        psz_Value;
    u16          u16_Len;
}VM_STRING, * PVM_STRING;

#define     VM_NULL         ((void*)0)
#define     VM_TRUE         1
#define     VM_FALSE        0

#ifndef caseretstr
#define caseretstr(x) case x: return #x
#endif  //caseretstr

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define UNUSED(x) (void)(x)

#include "vmdebug.h"

#endif     //PLATFORM_H