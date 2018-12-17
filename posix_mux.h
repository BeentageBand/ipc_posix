#ifndef POSIX_MUX_H_
#define POSIX_MUX_H_

#include <pthread.h>
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union POSIX_Mutex
{
    union POSIX_Mutex_Class _private * _private vtbl;
    struct
    {
        union Mutex_Cbk Mutex_Cbk;
        pthread_mutex_t mux;
    };
    struct Object Object;
}POSIX_Mutex_T;

typedef union Cygwin_Mutex
{
    union Cygwin_Mutex_Class _private * _private vtbl;
    struct
    {
        union Mutex_Cbk Mutex_Cbk;
        bool is_locked;
        pthread_mutex_t mux;
        pthread_cond_t cv;
    };
    struct Object Object;
}Cygwin_Mutex_T;

typedef union POSIX_Mutex_Class
{
    struct Mutex_Cbk_Class Mutex_Cbk;
    struct Class Class;
}POSIX_Mutex_Class_T;

typedef union Cygwin_Mutex_Class
{
    struct Mutex_Cbk_Class Mutex_Cbk;
    struct Class Class;
}Cygwin_Mutex_Class_T;

extern union POSIX_Mutex_Class _private POSIX_Mutex_Class;
extern union Cygwin_Mutex_Class _private Cygwin_Mutex_Class;

extern void Populate_POSIX_Mutex(union POSIX_Mutex * const cbk);
extern void Populate_Cygwin_Mutex(union Cygwin_Mutex * const cbk);;

#ifdef __cplusplus
}
#endif
#endif /*POSIX_MUX_H_*/
