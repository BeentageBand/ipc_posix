#ifndef POSIX_MBX_H_
#define POSIX_MBX_H_

#include "mailbox.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union POSIX_Mailbox
{
      union POSIX_Mailbox_Class _private * _private vtbl;
      union Mailbox_Cbk Mailbox_Cbk;
      struct Object Object;
}POSIX_Mailbox_T;

typedef union POSIX_Mailbox_Class 
{
      struct Class Class;
      struct Mailbox_Cbk_Class Mailbox_Cbk;
}POSIX_Mailbox_Class_T;

extern union POSIX_Mailbox_Class _private POSIX_Mailbox_Class;

extern void Populate_POSIX_Mailbox(union POSIX_Mailbox * const cbk);

#ifdef __cplusplus
}
#endif
#endif /*POSIX_MBX_H_*/