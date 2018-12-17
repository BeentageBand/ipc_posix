#define COBJECT_IMPLEMENTATION
#define Dbg_FID IPC_FID, 0
#include "dbg_log.h"
#include "ipc_posix.h"
#include "posix_thread.h"

typedef struct POSIX_Thread_Pair
{
    pthread_t pthread;
    IPC_TID_T tid;
}POSIX_Thread_Pair_T;

#define CSet_Params POSIX_Thread_Pair
#include "cset.h"
#include "cset.c"
#undef CSet_Params 

static int posix_thread_cmp(struct POSIX_Thread_Pair * a, struct POSIX_Thread_Pair * b);
static void * posix_thread_routine(void * thread);
static void posix_thread_delete(struct Object * const obj);
static bool posix_thread_register_thread(union Thread_Cbk * const cbk, union Thread * const thread );
static bool posix_thread_run_thread(union Thread_Cbk * const cbk, union Thread * const thread );
static bool posix_thread_join_thread(union Thread_Cbk * const cbk, union Thread * const thread );
static bool posix_thread_unregister_thread(union Thread_Cbk * const cbk, union Thread * const thread );

union POSIX_Thread_Class _private POSIX_Thread_Class = 
{
    {
        {posix_thread_delete, NULL},
        posix_thread_register_thread,
        posix_thread_run_thread,
        posix_thread_join_thread,
        posix_thread_unregister_thread
    }
};

static union POSIX_Thread POSIX_Thread = {NULL};
static CSet_POSIX_Thread_Pair_T POSIX_Thread_PThread_Pool = {NULL};
static POSIX_Thread_Pair_T POSIX_Thread_PThread_Buff[64] = {0};
static pthread_attr_t POSIX_Thread_Attr;
static pthread_mutexattr_t POSIX_Mux_Attr;

int posix_thread_cmp(struct POSIX_Thread_Pair * a, struct POSIX_Thread_Pair * b)
{
    return a->pthread - b->pthread;
}

void * posix_thread_routine(void * thread)
{
   union Thread * const this = _cast(Thread, (union Thread *)thread);

   Isnt_Nullptr(this, NULL);

   Dbg_Info("%s for thread = %d", __func__, this->tid);

   if(this->vtbl && this->vtbl->runnable)
   {
      this->vtbl->runnable(this);
   }
   else
   {
      Dbg_Fault("%s:Unable to run thread %d", __func__, this->tid);
   }

   pthread_exit(NULL);
   return NULL;
}

void posix_thread_delete(struct Object * const obj)
{
    union POSIX_Thread * const this = (union POSIX_Thread *)Object_Cast(&POSIX_Thread_Class.Class, obj);
    Isnt_Nullptr(this, );
    pthread_cancel(this->pthread);
}

IPC_TID_T ipc_posix_self_thread(union IPC_Helper * const helper)
{ 
    POSIX_Thread_Pair_T pair = {pthread_self(), 0};

    POSIX_Thread_Pair_T * const found = POSIX_Thread_PThread_Pool.vtbl->find(&POSIX_Thread_PThread_Pool, pair);
    IPC_TID_T tid = IPC_MAX_TID;

    if(POSIX_Thread_PThread_Pool.vtbl->end(&POSIX_Thread_PThread_Pool) != found)
    {
        tid  = found->tid;
    }
    return tid;
}

bool posix_thread_register_thread(union Thread_Cbk * const cbk, union Thread * const thread)
{
    union POSIX_Thread * const this = _cast(POSIX_Thread, cbk);
    Isnt_Nullptr(this, false);
    union IPC_Helper * const ipc_helper = IPC_get_instance();
    Isnt_Nullptr(ipc_helper, false);
    union Mutex * mux = ipc_helper->single_mux;
    Isnt_Nullptr(mux, false);

    if(!mux->vtbl->lock(mux, 200)) return false;
    CSet_Thread_Ptr_T * const thread_set = ipc_helper->rthreads;

    thread_set->vtbl->insert(thread_set, thread);
    mux->vtbl->unlock(mux);
    return NULL != IPC_Helper_find_thread(thread->tid);
}

bool posix_thread_run_thread(union Thread_Cbk * const cbk, union Thread * const thread)
{
    union POSIX_Thread * const this = _cast(POSIX_Thread, cbk);
    Isnt_Nullptr(this, false);
    bool rc = false;
    if((pthread_t)-1 == this->pthread)
    {
        rc = 0 == pthread_create(&this->pthread,
                                &POSIX_Thread_Attr,
                                posix_thread_routine,
                                (void*)thread);
        POSIX_Thread_Pair_T pair = {this->pthread, thread->tid};
        POSIX_Thread_PThread_Pool.vtbl->insert(&POSIX_Thread_PThread_Pool, pair);
    }
    return rc;
}

bool posix_thread_join_thread(union Thread_Cbk * const cbk, union Thread * const thread)
{
    union POSIX_Thread * const this = _cast(POSIX_Thread, cbk);
    Isnt_Nullptr(this, false);
    bool rc = 0 == pthread_join(this->pthread, NULL);
    POSIX_Thread_Pair_T pair = {this->pthread, thread->tid};
    POSIX_Thread_PThread_Pool.vtbl->erase(&POSIX_Thread_PThread_Pool, pair);
    return rc;
}

bool posix_thread_unregister_thread(union Thread_Cbk * const cbk, union Thread * const thread)
{
    union POSIX_Thread * const this = _cast(POSIX_Thread, cbk);
    Isnt_Nullptr(this, false);
    union IPC_Helper * const ipc_helper = IPC_get_instance();
    Isnt_Nullptr(ipc_helper, false);
    union Mutex * mux = ipc_helper->single_mux;
    Isnt_Nullptr(mux, false);
    union Thread * t_found = IPC_Helper_find_thread(thread->tid);
    Isnt_Nullptr(t_found, true);

    if(!mux->vtbl->lock(mux, 200)) return false;
    CSet_Thread_Ptr_T * const thread_set = ipc_helper->rthreads;
    thread_set->vtbl->erase(thread_set, thread);
    mux->vtbl->unlock(mux);
    return NULL == IPC_Helper_find_thread(thread->tid);
}

void Populate_POSIX_Thread(union POSIX_Thread * const this)
{
    if(NULL == POSIX_Thread.vtbl)
    {
        POSIX_Thread.Thread_Cbk.vtbl = &Thread_Cbk_Class;
        Object_Init(&POSIX_Thread.Object,
        &POSIX_Thread_Class.Class,
        sizeof(POSIX_Thread_Class.Thread_Cbk));
        pthread_attr_init(&POSIX_Thread_Attr);
        POSIX_Thread.pthread = (pthread_t)-1;
        Populate_CSet_Cmp_POSIX_Thread_Pair(&POSIX_Thread_PThread_Pool,
                    POSIX_Thread_PThread_Buff,
                    Num_Elems(POSIX_Thread_PThread_Buff), 
                    (CSet_Cmp_T)posix_thread_cmp);
        POSIX_Thread_Class.Thread_Cbk.register_thread = posix_thread_register_thread;
        POSIX_Thread_Class.Thread_Cbk.unregister_thread = posix_thread_unregister_thread;
        POSIX_Thread_Class.Thread_Cbk.join_thread = posix_thread_join_thread;
        POSIX_Thread_Class.Thread_Cbk.run_thread = posix_thread_run_thread;
    }
    _clone(this, POSIX_Thread);
	this->pthread  = (pthread_t)-1;
}
