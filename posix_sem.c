#define COBJECT_IMPLEMENTATION
#define Dbg_FID IPC_FID, 11
#include "dbg_log.h"
#include "ipc_posix.h"
#include "posix_sem.h"

static void posix_semaphore_delete(struct Object * const obj);
static bool posix_semaphore_wait(union Semaphore_Cbk * const cbk, union Semaphore * const sem, IPC_Clock_T const wait_ms);
static bool posix_semaphore_post(union Semaphore_Cbk * const cbk, union Semaphore * const sem);

static union POSIX_Semaphore POSIX_Semaphore = {NULL};

union POSIX_Semaphore_Class POSIX_Semaphore_Class = 
{{
    {posix_semaphore_delete, NULL},
    posix_semaphore_wait,
    posix_semaphore_post
}};

void posix_semaphore_delete(struct Object * const obj)
{
    union POSIX_Semaphore * const this = (union POSIX_Semaphore *)Object_Cast(&POSIX_Semaphore_Class.Class, obj);
    Isnt_Nullptr(this, );
    sem_destroy(&this->sem);
}

bool posix_semaphore_wait(union Semaphore_Cbk * const cbk, union Semaphore * const sem, IPC_Clock_T const wait_ms)
{
    union POSIX_Semaphore * const this = _cast(POSIX_Semaphore, cbk);
    Isnt_Nullptr(this, false);

    struct timespec wait_ts;
    ipc_posix_make_timespec(&wait_ts, wait_ms);
    return 0 == sem_timedwait(&this->sem, &wait_ts);
}

bool posix_semaphore_post(union Semaphore_Cbk * const cbk, union Semaphore * const sem)
{
    union POSIX_Semaphore * const this = _cast(POSIX_Semaphore, cbk);
    Isnt_Nullptr(this, false);

    return 0 == sem_post(&this->sem);
}

void Populate_POSIX_Semaphore(union POSIX_Semaphore * const this, uint32_t const resources)
{
    if(NULL == POSIX_Semaphore.vtbl)
    {
        POSIX_Semaphore.Semaphore_Cbk.vtbl = &Semaphore_Cbk_Class;
        Object_Init(&POSIX_Semaphore.Object,
                    &POSIX_Semaphore_Class.Class,
                    sizeof(POSIX_Semaphore_Class.Semaphore_Cbk));
    }
    _clone(this, POSIX_Semaphore);
    sem_init(&this->sem, 0, resources);
}
