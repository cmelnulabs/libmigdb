/**[txh]********************************************************************

  Copyright (c) 2026

  Module: Catchpoint Implementation
  Comments:
  Implementation of catchpoint support using CLI commands

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mi_gdb.h"
#include "catchpoint.h"

/* Helper to send catch command and parse response */
static mi_catch *send_catch_command(mi_h *h, const char *command)
{
 mi_catch *catch_point = NULL;
 mi_bkpt *bkpt = NULL;
 
 /* Send command */
 mi_send(h, "-interpreter-exec console \"%s\"\n", command);
 
 /* Get response - catchpoints are returned like breakpoints */
 if (!mi_get_response(h))
   return NULL;
 
 /* Try to parse as breakpoint response */
 bkpt = mi_res_bkpt(h);
 if (bkpt) {
   catch_point = mi_alloc_catch();
   if (catch_point) {
     catch_point->number = bkpt->number;
     catch_point->enabled = bkpt->enabled;
     catch_point->times = bkpt->times;
     catch_point->condition = bkpt->cond ? strdup(bkpt->cond) : NULL;
     catch_point->temporary = (bkpt->disp == d_del);
   }
   mi_free_bkpt(bkpt);
 }
 
 return catch_point;
}

mi_catch *gmi_catch_exception(mi_h *h, int is_throw, int temporary)
{
 char cmd[256];
 
 if (!h)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "%scatch %s", 
          temporary ? "t" : "", 
          is_throw ? "throw" : "catch");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c)
   c->type = is_throw ? mi_catch_throw : mi_catch_catch;
 
 return c;
}

mi_catch *gmi_catch_exec(mi_h *h, int temporary)
{
 char cmd[256];
 
 if (!h)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "%scatch exec", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c)
   c->type = mi_catch_exec;
 
 return c;
}

mi_catch *gmi_catch_fork(mi_h *h, int is_vfork, int temporary)
{
 char cmd[256];
 
 if (!h)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "%scatch %s", 
          temporary ? "t" : "",
          is_vfork ? "vfork" : "fork");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c)
   c->type = is_vfork ? mi_catch_vfork : mi_catch_fork;
 
 return c;
}

mi_catch *gmi_catch_load(mi_h *h, const char *regexp, int temporary)
{
 char cmd[512];
 
 if (!h)
   return NULL;
 
 if (regexp)
   snprintf(cmd, sizeof(cmd), "%scatch load %s", temporary ? "t" : "", regexp);
 else
   snprintf(cmd, sizeof(cmd), "%scatch load", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c) {
   c->type = mi_catch_load;
   if (regexp)
     c->event = strdup(regexp);
 }
 
 return c;
}

mi_catch *gmi_catch_unload(mi_h *h, const char *regexp, int temporary)
{
 char cmd[512];
 
 if (!h)
   return NULL;
 
 if (regexp)
   snprintf(cmd, sizeof(cmd), "%scatch unload %s", temporary ? "t" : "", regexp);
 else
   snprintf(cmd, sizeof(cmd), "%scatch unload", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c) {
   c->type = mi_catch_unload;
   if (regexp)
     c->event = strdup(regexp);
 }
 
 return c;
}

mi_catch *gmi_catch_syscall(mi_h *h, const char *syscall_name, int temporary)
{
 char cmd[512];
 
 if (!h)
   return NULL;
 
 if (syscall_name)
   snprintf(cmd, sizeof(cmd), "%scatch syscall %s", temporary ? "t" : "", syscall_name);
 else
   snprintf(cmd, sizeof(cmd), "%scatch syscall", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c) {
   c->type = mi_catch_syscall;
   if (syscall_name)
     c->event = strdup(syscall_name);
 }
 
 return c;
}

mi_catch *gmi_catch_signal(mi_h *h, const char *signal_name, int temporary)
{
 char cmd[512];
 
 if (!h)
   return NULL;
 
 if (signal_name)
   snprintf(cmd, sizeof(cmd), "%scatch signal %s", temporary ? "t" : "", signal_name);
 else
   snprintf(cmd, sizeof(cmd), "%scatch signal", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c) {
   c->type = mi_catch_signal;
   if (signal_name)
     c->event = strdup(signal_name);
 }
 
 return c;
}

mi_catch *gmi_catch_assert(mi_h *h, int temporary)
{
 char cmd[256];
 
 if (!h)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "%scatch assert", temporary ? "t" : "");
 
 mi_catch *c = send_catch_command(h, cmd);
 if (c)
   c->type = mi_catch_assert;
 
 return c;
}

int gmi_catch_delete(mi_h *h, int number)
{
 /* Catchpoints are deleted same as breakpoints */
 return gmi_break_delete(h, number);
}

int gmi_catch_state(mi_h *h, int number, int enable)
{
 /* Catchpoints are enabled/disabled same as breakpoints */
 return gmi_break_state(h, number, enable);
}

int gmi_catch_set_condition(mi_h *h, int number, const char *condition)
{
 /* Catchpoints support conditions same as breakpoints */
 return gmi_break_set_condition(h, number, condition);
}

mi_catch *mi_alloc_catch(void)
{
 return (mi_catch *)mi_calloc1(sizeof(mi_catch));
}

void mi_free_catch(mi_catch *c)
{
 if (!c) return;
 free(c->condition);
 free(c->event);
 mi_free_catch(c->next);
 free(c);
}
