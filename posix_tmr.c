#define COBJECT_IMPLEMENTATION
#define Dbg_FID IPC_FID, 9

#include "dbg_log.h"
#include "ipc.h"
#include "ipc_posix.h"
#include "posix_tmr.h"

static void posix_timer_on_tout(int sig, siginfo_t * siginfo, void * params);
static void posix_timer_delete(struct Object * const obj);
static bool posix_timer_start(union Timer_Cbk * const, union Timer * const);
static bool posix_timer_stop(union Timer_Cbk * const, union Timer * const);

static union POSIX_Timer POSIX_Timer = {NULL};

union POSIX_Timer_Class POSIX_Timer_Class = 
{{
        {posix_timer_delete, NULL},
        posix_timer_start,
        posix_timer_stop
}};

void posix_timer_on_tout(int sig, siginfo_t * siginfo, void * params)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL); // block signal mask

    union Timer * const timer = (union Timer *)Object_Cast(&Timer_Class.Class,(struct Object *) params);
    if(NULL == timer) return; //No dbg log because dbg log is not thread safe.
    timer->vtbl->on_tout(timer);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void posix_timer_delete(struct Object * const obj)
{
    union POSIX_Timer * const this = (union POSIX_Timer *)Object_Cast(&POSIX_Timer_Class.Class, obj);
    Isnt_Nullptr(this, );
    timer_delete(&this->tmrid);
}

bool posix_timer_start(union Timer_Cbk * const cbk, union Timer * const timer)
{
    union POSIX_Timer * const this = _cast(POSIX_Timer, cbk);
    Isnt_Nullptr(this, false);
    struct itimerspec itimer;

    if(timer->is_periodic)
    {
        ipc_posix_make_timespec(&itimer.it_interval, timer->tout_ms);
        memset(&itimer.it_value,  0, sizeof(itimer.it_value));
    }
    else
    {
        ipc_posix_make_timespec(&itimer.it_value, timer->tout_ms);
        memset(&itimer.it_interval,  0, sizeof(itimer.it_interval));
    }
    return 0 == timer_settime(this->tmrid, 0, &itimer, NULL);
}

bool posix_timer_stop(union Timer_Cbk * const cbk, union Timer * const tmr)
{
    union POSIX_Timer * const this = _cast(POSIX_Timer, cbk);
    Isnt_Nullptr(this, false);
    struct itimerspec itimer;
    memset(&itimer,  0, sizeof(itimer));
    return 0 == timer_settime(&this->tmrid, 0, &itimer, NULL);
}

void Populate_POSIX_Timer(union POSIX_Timer * const this, union Timer * const timer)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL); // block signal mask

    if(NULL == POSIX_Timer.vtbl)
    {
        POSIX_Timer.Timer_Cbk.vtbl = &Timer_Cbk_Class;
        Object_Init(&POSIX_Timer.Object,
                &POSIX_Timer_Class.Class,
                sizeof(POSIX_Timer_Class.Timer_Cbk));
        POSIX_Timer.sigaction.sa_flags = SA_SIGINFO;
        POSIX_Timer.sigaction.sa_sigaction = posix_timer_on_tout;

        sigemptyset(&POSIX_Timer.sigaction.sa_mask);
        POSIX_Timer.sigevent.sigev_notify = SIGEV_SIGNAL;
        POSIX_Timer.sigevent.sigev_signo = SIGRTMIN;
    }
    _clone(this, POSIX_Timer);

    sigaction(SIGRTMIN, &POSIX_Timer.sigaction, NULL);
    POSIX_Timer.sigevent.sigev_value.sival_ptr = timer;


    timer_create(CLOCK_MONOTONIC, &this->sigevent, &this->tmrid);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
