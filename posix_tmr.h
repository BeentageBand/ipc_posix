#ifndef POSIX_TMR_H_
#define POSIX_TMR_H_

#include <time.h>
#include <signal.h>
#include "tmr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union POSIX_Timer
{
    union POSIX_Timer_Class _private * _private vtbl;
    struct
    {
        union Timer_Cbk Timer_Cbk;
		struct sigevent sigevent;
		struct sigaction sigaction;
        timer_t tmrid;
    };
    struct Object Object;
}POSIX_Timer_T;

typedef union POSIX_Timer_Class
{
    struct Timer_Cbk_Class Timer_Cbk;
    struct Class Class;
}POSIX_Timer_Class_T;

extern union POSIX_Timer_Class _private POSIX_Timer_Class;

extern void Populate_POSIX_Timer(union POSIX_Timer * const cbk, union Timer * const timer);

#ifdef __cplusplus
}
#endif

#endif /*POSIX_TMR_H_*/
