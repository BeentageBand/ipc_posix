#define COBJECT_IMPLEMENTATION
#include "posix_mbx.h"
#include "ipc_helper.h"

static void posix_mailbox_delete(struct Object * const obj);

union POSIX_Mailbox_Class POSIX_Mailbox_Class  = 
{
    {posix_mailbox_delete, NULL},
    NULL,
    NULL
};

static union POSIX_Mailbox POSIX_Mailbox = {NULL};

void posix_mailbox_delete(struct Object * const obj){}

void Populate_POSIX_Mailbox(union POSIX_Mailbox * const this)
{
    if(NULL ==  POSIX_Mailbox.vtbl)
    {
        POSIX_Mailbox.Mailbox_Cbk.vtbl = &Mailbox_Cbk_Class;
        Object_Init(&POSIX_Mailbox.Object,
                    &POSIX_Mailbox_Class.Class,
                    sizeof(POSIX_Mailbox_Class.Mailbox_Cbk));
    }
    _clone(this, POSIX_Mailbox);
}