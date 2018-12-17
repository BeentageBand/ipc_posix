#define COBJECT_IMPLEMENTATION
#define Dbg_FID IPC_FID, 8
#include "dbg_log.h"
#include "ipc_posix.h"
#include "posix_mux.h"

static void posix_mutex_delete(struct Object * const obj);
static bool posix_mutex_lock(union Mutex_Cbk * const, union Mutex * const, IPC_Clock_T const wait_ms);
static bool posix_mutex_unlock(union Mutex_Cbk * const, union Mutex * const);

static void cygwin_mutex_delete(struct Object * const obj);
static bool cygwin_mutex_lock(union Mutex_Cbk * const, union Mutex * const, IPC_Clock_T const wait_ms);
static bool cygwin_mutex_unlock(union Mutex_Cbk * const, union Mutex * const);

union POSIX_Mutex_Class POSIX_Mutex_Class =
{{
        {posix_mutex_delete, NULL},
        posix_mutex_lock,
        posix_mutex_unlock
}};

union Cygwin_Mutex_Class Cygwin_Mutex_Class =
{{
        {cygwin_mutex_delete, NULL},
        cygwin_mutex_lock,
        cygwin_mutex_unlock
}};

static union POSIX_Mutex POSIX_Mutex = {NULL};
static pthread_mutexattr_t POSIX_Mux_Attr;

static union Cygwin_Mutex Cygwin_Mutex = {NULL};
static pthread_mutexattr_t Cygwin_Mux_Attr;
static pthread_condattr_t Cygwin_CV_Attr;

void posix_mutex_delete(struct Object * const obj)
{
    union POSIX_Mutex * const this = (union POSIX_Mutex *)Object_Cast(&POSIX_Mutex_Class.Class, obj);
    Isnt_Nullptr(this, );
    pthread_mutex_destroy(&this->mux);
}

bool posix_mutex_lock(union Mutex_Cbk * const cbk, union Mutex * const mux, IPC_Clock_T const wait_ms)
{
#ifdef _POSIX_TIMEOUTS
    union POSIX_Mutex * const this = _cast(POSIX_Mutex, cbk);
    Isnt_Nullptr(this, false);
    struct timespec wait_ts;
    ipc_posix_make_timespec(&wait_ts, wait_ms);
    return 0 == pthread_timedlock(&this->mux, &wait_ts);
#else
    return false;
#endif
}

bool posix_mutex_unlock(union Mutex_Cbk * const cbk, union Mutex * const mux)
{
    union POSIX_Mutex * const this = _cast(POSIX_Mutex, cbk);
    Isnt_Nullptr(this, false);
    return 0 == pthread_mutex_unlock(&this->mux);
}

void cygwin_mutex_delete(struct Object * const obj)
{
    union Cygwin_Mutex * const this = (union Cygwin_Mutex *)Object_Cast(&Cygwin_Mutex_Class.Class, obj);
    Isnt_Nullptr(this, );
    pthread_mutex_destroy(&this->mux);
    pthread_cond_destroy(&this->cv);
}

bool cygwin_mutex_lock(union Mutex_Cbk * const cbk, union Mutex * const mutex, IPC_Clock_T const wait_ms)
{
    union Cygwin_Mutex * const this = _cast(Cygwin_Mutex, cbk);
    Isnt_Nullptr(this, false);
    if(pthread_mutex_lock(&this->mux)) return false;
    struct timespec wait_ts;
    ipc_posix_make_timespec(&wait_ts, wait_ms);
    while(this->is_locked)
    {
        int rc = pthread_cond_timedwait(&this->cv, &this->mux, &wait_ts);
        if(rc < 0) return false;
    }
    this->is_locked = true;
    return 0 == pthread_mutex_unlock(&this->mux);
}

bool cygwin_mutex_unlock(union Mutex_Cbk * const cbk, union Mutex * const mutex)
{
    union Cygwin_Mutex * const this = _cast(Cygwin_Mutex, cbk);
    Isnt_Nullptr(this, false);
    if(pthread_mutex_lock(&this->mux)) return false;
    this->is_locked = false;
    pthread_cond_signal(&this->cv);
    return 0 == pthread_mutex_unlock(&this->mux);
}

void Populate_POSIX_Mutex(union POSIX_Mutex * const this)
{
    if(NULL == POSIX_Mutex.vtbl)
    {
        POSIX_Mutex.Mutex_Cbk.vtbl = &Mutex_Cbk_Class;
        Object_Init(&POSIX_Mutex.Object,
                &POSIX_Mutex_Class.Class,
                sizeof(POSIX_Mutex_Class.Class));
        pthread_mutexattr_init(&POSIX_Mux_Attr);
        POSIX_Mutex_Class.Mutex_Cbk.lock = posix_mutex_lock;
        POSIX_Mutex_Class.Mutex_Cbk.unlock = posix_mutex_unlock;
    }
    _clone(this, POSIX_Mutex);
    pthread_mutex_init(&this->mux, &POSIX_Mux_Attr);
}

void Populate_Cygwin_Mutex(union Cygwin_Mutex * const this)
{
    if(NULL == Cygwin_Mutex.vtbl)
    {
        Cygwin_Mutex.Mutex_Cbk.vtbl = &Mutex_Cbk_Class;
        Object_Init(&Cygwin_Mutex.Object,
                &Cygwin_Mutex_Class.Class,
                sizeof(Cygwin_Mutex_Class.Class));
        Cygwin_Mutex.is_locked = false;

        Cygwin_Mutex_Class.Mutex_Cbk.lock = cygwin_mutex_lock;
        Cygwin_Mutex_Class.Mutex_Cbk.unlock = cygwin_mutex_unlock;
        pthread_mutexattr_init(&Cygwin_Mux_Attr);
        pthread_condattr_init(&Cygwin_CV_Attr);
    }
    _clone(this, Cygwin_Mutex);
    pthread_mutex_init(&this->mux, &Cygwin_Mux_Attr);
    pthread_cond_init(&this->cv, &Cygwin_CV_Attr);
}
