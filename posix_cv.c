#define COBJECT_IMPLEMENTATION
#define Dbg_FID IPC_FID, 10
#include "dbg_log.h"
#include "ipc_posix.h"
#include "posix_mux.h"
#include "posix_cv.h"

static void posix_conditional_delete(struct Object * const obj);
static bool posix_conditional_wait(union Conditional_Cbk * const,
            union Conditional * const, IPC_Clock_T const wait_ms);
static bool posix_conditional_post(union Conditional_Cbk * const, union Conditional * const);

static union POSIX_Conditional POSIX_Conditional = {NULL};

union POSIX_Conditional_Class POSIX_Conditional_Class = 
{
    {
        {posix_conditional_delete, NULL},
        posix_conditional_wait,
        posix_conditional_post
    }
};

void posix_conditional_delete(struct Object * const obj)
{
    union POSIX_Conditional * const this =
     (union POSIX_Conditional *)Object_Cast(&POSIX_Conditional_Class.Class, obj);
    Isnt_Nullptr(this, );

    pthread_cond_destroy(&this->cv);
}
bool posix_conditional_wait(union Conditional_Cbk * const cbk,
            union Conditional * const conditional, IPC_Clock_T const wait_ms)
{
    union POSIX_Conditional * const this = _cast(POSIX_Conditional, cbk);
    Isnt_Nullptr(this, false);
    Isnt_Nullptr(conditional->mutex, false);
    union POSIX_Mutex * const mux = _cast(POSIX_Mutex, conditional->mutex->cbk);
    Isnt_Nullptr(mux, false);
    struct timespec wait_ts;
    ipc_posix_make_timespec(&wait_ts, wait_ms);
    return 0 == pthread_cond_timedwait(&this->cv, &mux->mux, &wait_ts);
}

bool posix_conditional_post(union Conditional_Cbk * const cbk, union Conditional * const conditional)
{
    union POSIX_Conditional * const this = _cast(POSIX_Conditional, cbk);
    Isnt_Nullptr(this, false);
    return 0 == pthread_cond_signal(&this->cv);
}

void Populate_POSIX_Conditional(union POSIX_Conditional * const this)
{
    if(NULL == POSIX_Conditional.vtbl)
    {
        POSIX_Conditional.Conditional_Cbk.vtbl = &Conditional_Cbk_Class;
        Object_Init(&POSIX_Conditional.Object,
                    &POSIX_Conditional_Class.Class,
                    sizeof(POSIX_Conditional_Class.Conditional_Cbk));
        POSIX_Conditional_Class.Conditional_Cbk.wait = posix_conditional_wait;
        POSIX_Conditional_Class.Conditional_Cbk.post = posix_conditional_post;
    }
    _clone(this, POSIX_Conditional);
}
