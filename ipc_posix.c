/*
 * ipc_posix.c
 *
 *  Created on: Jan 27, 2018
 *      Author: uids6144
 */

#define COBJECT_IMPLEMENTATION
#define Dbg_FID DBG_FID_DEF(IPC_FID, 4)

#include <time.h>
#include "dbg_log.h"
#include "ipc.h"
#include "ipc_posix.h"
#include "posix_cv.h"
#include "posix_mbx.h"
#include "posix_mux.h"
#include "posix_sem.h"
#include "posix_thread.h"
#include "posix_tmr.h"

static void ipc_posix_delete(struct Object * const obj);

static IPC_Clock_T ipc_posix_time(union IPC_Helper * const helper);
static void ipc_posix_sleep(union IPC_Helper * const helper, IPC_Clock_T const sleep_ms);

static bool ipc_posix_alloc_thread(union IPC_Helper * const helper, union Thread * const thread);
static bool ipc_posix_alloc_mailbox(union IPC_Helper * const helper, union Mailbox * const mailbox);
static bool ipc_posix_alloc_mutex(union IPC_Helper * const helper, union Mutex * const mutex);
static bool ipc_posix_alloc_semaphore(union IPC_Helper * const helper, union Semaphore * const semaphore,
                  uint8_t const value);
static bool ipc_posix_alloc_conditional(union IPC_Helper * const helper, union Conditional * const conditional);
static bool ipc_posix_alloc_timer(union IPC_Helper * const helper, union Timer * const timer);

IPC_POSIX_Class_T IPC_POSIX_Class =
    {{
   { ipc_posix_delete, NULL},
   ipc_posix_time,
   ipc_posix_sleep,
   ipc_posix_self_thread,

   ipc_posix_alloc_thread,
   ipc_posix_alloc_mailbox,
   ipc_posix_alloc_mutex,
   ipc_posix_alloc_semaphore,
   ipc_posix_alloc_conditional,
   ipc_posix_alloc_timer
    }};

static union IPC_POSIX IPC_POSIX = {NULL};

void ipc_posix_delete(struct Object * const obj)
{}

IPC_Clock_T ipc_posix_time(union IPC_Helper * const helper)
{
  struct timespec timespec;
  int rc = clock_gettime(CLOCK_MONOTONIC, &timespec);
  return (rc)? 0 : (timespec.tv_sec * 1000 + timespec.tv_nsec / 1000000);
}

void ipc_posix_sleep(union IPC_Helper * const helper, IPC_Clock_T const sleep_ms)
{
  struct timespec sleep_ts;
  ipc_posix_make_timespec(&sleep_ts, sleep_ms);
  nanosleep(&sleep_ts, NULL);
}

bool ipc_posix_alloc_thread(union IPC_Helper * const helper, union Thread * const thread)
{
  bool rc = true;
  union POSIX_Thread * const posix_thread = (union POSIX_Thread *)malloc(sizeof(union POSIX_Thread));
  Isnt_Nullptr(posix_thread, false);
  memset(posix_thread, 0, sizeof(union POSIX_Thread));
  Populate_POSIX_Thread(posix_thread);
  thread->cbk = &posix_thread->Thread_Cbk;
  return rc;
}

bool ipc_posix_alloc_mailbox(union IPC_Helper * const helper, union Mailbox * const mailbox)
{
  bool rc = true;
  union POSIX_Mailbox * const posix_mbx = (union POSIX_Mailbox *)malloc(sizeof(union POSIX_Mailbox));
  Isnt_Nullptr(posix_mbx, false);
  memset(posix_mbx, 0, sizeof(union POSIX_Mailbox));
  Populate_POSIX_Mailbox(posix_mbx);
  mailbox->cbk = &posix_mbx->Mailbox_Cbk;
  return rc;
}

#ifdef _POSIX_TIMEOUTS
bool ipc_posix_alloc_mutex(union IPC_Helper * const helper, union Mutex * const mutex)
{
  union POSIX_Mutex * const posix_mux = (union POSIX_Mutex *)malloc(sizeof(union POSIX_Mutex));
  Isnt_Nullptr(posix_mux, false);
  memset(posix_mux, 0, sizeof(union POSIX_Mutex));
  Populate_POSIX_Mutex(posix_mux);
  mutex->cbk = &posix_mux->Mutex_Cbk;
  return NULL != mutex->cbk;
}

#else
bool ipc_posix_alloc_mutex(union IPC_Helper * const helper, union Mutex * const mutex)
{
  union Cygwin_Mutex * const cygwin_mux = (union Cygwin_Mutex *)malloc(sizeof(union Cygwin_Mutex));
  Isnt_Nullptr(cygwin_mux, false);
  memset(cygwin_mux, 0, sizeof(union Cygwin_Mutex));
  Populate_Cygwin_Mutex(cygwin_mux);
  mutex->cbk = &cygwin_mux->Mutex_Cbk;
  return NULL != mutex->cbk; 
}

#endif

bool ipc_posix_alloc_semaphore(union IPC_Helper * const helper, union Semaphore * const semaphore,
                  uint8_t const value)
{
	union POSIX_Semaphore * const posix_sem = (union POSIX_Semaphore *)malloc(sizeof(union POSIX_Semaphore));
	Isnt_Nullptr(posix_sem, false);
  memset(posix_sem, 0, sizeof(union POSIX_Semaphore));
	Populate_POSIX_Semaphore(posix_sem, value);
  semaphore->cbk = &posix_sem->Semaphore_Cbk;
	return NULL != semaphore->cbk;
}

bool ipc_posix_alloc_conditional(union IPC_Helper * const helper, union Conditional * const conditional)
{
  union POSIX_Conditional * const posix_cv = (union POSIX_Conditional *)malloc(sizeof(union POSIX_Conditional));
  Isnt_Nullptr(posix_cv, false);
  memset(posix_cv, 0, sizeof(union POSIX_Conditional));
  Populate_POSIX_Conditional(posix_cv);
  conditional->cbk = &posix_cv->Conditional_Cbk;
  return NULL != conditional->cbk;
}

bool ipc_posix_alloc_timer(union IPC_Helper * const helper, union Timer * const timer)
{
  union POSIX_Timer * const posix_timer = (union POSIX_Timer *)malloc(sizeof(union POSIX_Timer));
  Isnt_Nullptr(posix_timer, false);
  memset(posix_timer, 0, sizeof(union POSIX_Timer));
  Populate_POSIX_Timer(posix_timer, timer);
  timer->cbk = &posix_timer->Timer_Cbk;
  return NULL != timer->cbk;
}

void ipc_posix_make_timespec(struct timespec * const tm, IPC_Clock_T const clock_ms)
{
  tm->tv_sec = clock_ms / 1000;
  tm->tv_nsec = clock_ms - (tm->tv_sec * 1000);
  tm->tv_nsec *= 1000000;
}

IPC_Clock_T ipc_posix_make_clock(struct timespec * const tm)
{
  return (IPC_Clock_T)(tm->tv_nsec / 1000000) + (IPC_Clock_T)(tm->tv_sec * 1000);
}

void Populate_IPC_POSIX(union IPC_POSIX * const this)
{
  if(NULL == IPC_POSIX.vtbl)
    {
      IPC_Helper_Append(&IPC_POSIX.IPC_Helper);
      Populate_IPC_Helper(&IPC_POSIX.IPC_Helper);
      Object_Init(&IPC_POSIX.Object, &IPC_POSIX_Class.Class, 0);
      Populate_Mutex(IPC_POSIX.IPC_Helper.single_mux);
    }
    _clone(this, IPC_POSIX);
}
