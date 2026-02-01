/**[txh]********************************************************************

  Copyright (c) 2026
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Module: Catchpoint Support
  Comments:
  Support for GDB catchpoints - breakpoints for events like exceptions,
  signals, system calls, etc.

***************************************************************************/

#ifndef MI_CATCHPOINT_H
#define MI_CATCHPOINT_H

#include "mi_gdb.h"

/* Catchpoint types */
typedef enum {
 mi_catch_throw,      /* C++ exception throw */
 mi_catch_catch,      /* C++ exception catch */
 mi_catch_exec,       /* exec() system call */
 mi_catch_fork,       /* fork() system call */
 mi_catch_vfork,      /* vfork() system call */
 mi_catch_load,       /* Shared library load */
 mi_catch_unload,     /* Shared library unload */
 mi_catch_syscall,    /* System call */
 mi_catch_signal,     /* Signal */
 mi_catch_assert,     /* Assertion failure */
} mi_catch_type;

/* Catchpoint structure */
typedef struct mi_catch_struct
{
 int number;
 mi_catch_type type;
 char enabled;
 char *condition;
 int times;          /* Hit count */
 char *event;        /* Event name (syscall name, signal name, etc.) */
 char temporary;
 
 struct mi_catch_struct *next;
} mi_catch;

/**
 * Set a catchpoint for C++ exceptions
 * @param h      GDB handle
 * @param is_throw  1 for throw, 0 for catch
 * @param temporary 1 for temporary catchpoint
 * @return Catchpoint structure or NULL on error
 */
mi_catch *gmi_catch_exception(mi_h *h, int is_throw, int temporary);

/**
 * Set a catchpoint for exec
 */
mi_catch *gmi_catch_exec(mi_h *h, int temporary);

/**
 * Set a catchpoint for fork/vfork
 */
mi_catch *gmi_catch_fork(mi_h *h, int is_vfork, int temporary);

/**
 * Set a catchpoint for shared library load/unload
 * @param regexp Regular expression for library name (NULL for all)
 */
mi_catch *gmi_catch_load(mi_h *h, const char *regexp, int temporary);
mi_catch *gmi_catch_unload(mi_h *h, const char *regexp, int temporary);

/**
 * Set a catchpoint for system call
 * @param syscall_name Name or number of syscall (NULL for all)
 */
mi_catch *gmi_catch_syscall(mi_h *h, const char *syscall_name, int temporary);

/**
 * Set a catchpoint for signal
 * @param signal_name Name or number of signal (NULL for all)
 */
mi_catch *gmi_catch_signal(mi_h *h, const char *signal_name, int temporary);

/**
 * Set a catchpoint for assertion failures
 */
mi_catch *gmi_catch_assert(mi_h *h, int temporary);

/**
 * Delete a catchpoint
 */
int gmi_catch_delete(mi_h *h, int number);

/**
 * Enable/disable a catchpoint
 */
int gmi_catch_state(mi_h *h, int number, int enable);

/**
 * Set condition on catchpoint
 */
int gmi_catch_set_condition(mi_h *h, int number, const char *condition);

/**
 * Memory management
 */
mi_catch *mi_alloc_catch(void);
void mi_free_catch(mi_catch *c);

#endif /* MI_CATCHPOINT_H */
