/*

  Copyright (c) 2016 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef DILL_CR_INCLUDED
#define DILL_CR_INCLUDED

#include <stdint.h>

#include "chan.h"
#include "debug.h"
#include "list.h"
#include "slist.h"
#include "timer.h"
#include "utils.h"

enum dill_state {
    DILL_READY,
    DILL_MSLEEP,
    DILL_FDWAIT,
    DILL_CHR,
    DILL_CHS,
    DILL_CHOOSE
};

/* The coroutine. The memory layout looks like this:

   +-------------------------------------------------------------+---------+
   |                                                      stack  | dill_cr |
   +-------------------------------------------------------------+---------+

   - dill_cr contains generic book-keeping info about the coroutine
   - stack is a standard C stack; it grows downwards (at the moment treestack
     doesn't support microarchitectures where stack grows upwards)

*/
struct dill_cr {
    /* The coroutine is stored in this list if it is not blocked and it is
       waiting to be executed. */
    struct dill_slist_item ready;

    /* If the coroutine is waiting for a deadline, it uses this timer. */
    struct dill_timer timer;

    /* When the coroutine is blocked in fdwait(), these members contains the
       file descriptor and events that the function waits for. They are used
       only for debugging purposes. */
    int fd;
    int events;

    /* This structure is used when the coroutine is executing a choose
       statement. */
    struct dill_choosedata choosedata;

    /* Stored coroutine context while it is not executing. */
    struct dill_ctx ctx;

    /* Argument to resume() call being passed to the blocked suspend() call. */
    int result;

    /* Coroutine-local storage. */
    void *cls;

    /* Debugging info. */
    struct dill_debug_cr debug;

    /* TODO: Following fields should go to dill_debug_cr. */
    /* Status of the coroutine. Used for debugging purposes. */
    enum dill_state state;

    /* If coroutine is stuck in choose(), here are the clauses.
       Used for debugging. */
    int nclauses;
    struct chclause *clauses;
};

/* Fake coroutine corresponding to the main thread of execution. */
extern struct dill_cr dill_main;

/* The coroutine that is running at the moment. */
extern struct dill_cr *dill_running;

/* Suspend running coroutine. Move to executing different coroutines. Once
   someone resumes this coroutine using dill_resume function 'result' argument
   of that function will be returned. */
int dill_suspend(void);

/* Schedules preiously suspended coroutine for execution. Keep in mind that
   it doesn't immediately run it, just puts it into the queue of ready
   coroutines. */
void dill_resume(struct dill_cr *cr, int result);

#endif