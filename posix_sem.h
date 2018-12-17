#ifndef POSIX_SEM_H_
#define POSIX_SEM_H_

#include <semaphore.h>
#include "sem.h"

typedef union POSIX_Semaphore
{
    union POSIX_Semaphore_Class _private * _private vtbl;
    struct
    {
        union Semaphore_Cbk Semaphore_Cbk;
        sem_t sem;
    };
    struct Object Object;
}POSIX_Semaphore_T;

typedef union POSIX_Semaphore_Class
{
    struct Semaphore_Cbk_Class Semaphore_Cbk;
    struct Class Class;
}POSIX_Semaphore_Class_T;

extern union POSIX_Semaphore_Class _private POSIX_Semaphore_Class;

extern void Populate_POSIX_Semaphore(union POSIX_Semaphore * const this, uint32_t const resources);

#endif /*POSIX_SEM_H_*/
