/******************************************************************************
* Copyright (c) 2015-2017 jiangxiaogang<kerndev@foxmail.com>
*
* This file is part of KLite distribution.
*
* KLite is free software, you can redistribute it and/or modify it under
* the MIT Licence.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/
#include "kernel.h"
#include "internal.h"
#include "list.h"

void kobject_init(struct object* obj)
{
    obj->head = NULL;
    obj->tail = NULL;
    obj->data = 0;
}

void kobject_wait(struct object* obj, struct tcb* tcb)
{
    tcb->state = TCB_STATE_WAIT;
    tcb->lwait = (struct tcb_list*)obj;
    ksched_insert((struct tcb_list*)obj, tcb->nwait);
}

void kobject_post(struct object* obj, struct tcb* tcb)
{
    if(tcb->lsched)
    {
        list_remove(tcb->lsched, tcb->nsched);
    }
    tcb->state  = TCB_STATE_READY;
    tcb->lwait  = NULL;
    tcb->lsched = &sched_list_ready;
    list_remove(obj, tcb->nwait);
    ksched_insert(&sched_list_ready, tcb->nsched);
}

void kobject_timedwait(struct object* obj, struct tcb* tcb, uint32_t timeout)
{
    tcb->timeout = timeout;
    tcb->state   = TCB_STATE_TIMEDWAIT;
    tcb->lwait   = (struct tcb_list*)obj;
    tcb->lsched  = &sched_list_sleep;
    list_append(&sched_list_sleep, tcb->nsched);
    ksched_insert((struct tcb_list*)obj, tcb->nwait);
}
