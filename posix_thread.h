#ifndef POSIX_THREAD_H_
#define POSIX_THREAD_H_

#include <pthread.h>
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union POSIX_Thread
{
    union POSIX_Thread_Class _private * _private vtbl;
    struct
    {
        union Thread_Cbk Thread_Cbk;
        pthread_t pthread;
    };
    struct Object Object;
}POSIX_Thread_T;

typedef union POSIX_Thread_Class 
{
    struct Thread_Cbk_Class Thread_Cbk;
    struct Class Class;
}POSIX_Thread_Class_T;

extern union POSIX_Thread_Class _private POSIX_Thread_Class;

extern void Populate_POSIX_Thread(union POSIX_Thread * const this);

#ifdef __cplusplus
}
#endif
#endif /*POSIX_THREAD_H_*/