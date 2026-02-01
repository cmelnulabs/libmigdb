/**[txh]********************************************************************

  Copyright (c) 2026
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Module: Extended Symbol Query Implementation
  Comments:
  Implementation of symbol query functions using CLI commands with
  console output parsing since MI commands are not available for
  most symbol operations.

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "mi_gdb.h"
#include "symbol_query_ext.h"

/* Helper: Send CLI command and capture console output */
static char *send_cli_command(mi_h *h, const char *command)
{
 char *result = NULL;
 
 if (!h || !command)
   return NULL;
 
 /* Enable console capture */
 h->catch_console = 1;
 h->catched_console = NULL;
 
 /* Send the command using -interpreter-exec */
 mi_send(h, "-interpreter-exec console \"%s\"\n", command);
 
 /* Wait for response */
 if (!mi_get_response(h)) {
   h->catch_console = 0;
   return NULL;
 }
 
 /* Get captured console output */
 result = h->catched_console;
 h->catched_console = NULL;
 h->catch_console = 0;
 
 return result;
}

/* Parse hex address from string */
static void *parse_address(const char *str)
{
 void *addr = NULL;
 if (str && (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')))
   sscanf(str, "%p", &addr);
 return addr;
}

/**
 * Get address of a symbol
 */
mi_symbol *gmi_symbol_info_address(mi_h *h, const char *symbol)
{
 char cmd[512];
 char *output;
 mi_symbol *sym = NULL;
 
 if (!h || !symbol)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "info address %s", symbol);
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 /* Parse output like: "Symbol \"foo\" is at 0x12345 in a file compiled without debugging." */
 /* or "Symbol \"bar\" is a variable at frame base reg $rbp offset -4." */
 
 sym = mi_alloc_symbol();
 if (!sym) {
   free(output);
   return NULL;
 }
 
 sym->name = strdup(symbol);
 if (!sym->name) {
   mi_free_symbol(sym);
   free(output);
   return NULL;
 }
 
 char *addr_str = strstr(output, "0x");
 if (addr_str) {
   char addr_buf[32];
   sscanf(addr_str, "%31s", addr_buf);
   sym->addr = parse_address(addr_buf);
 }
 
 /* Extract more info if available */
 if (strstr(output, "is a function"))
   sym->type = strdup("function");
 else if (strstr(output, "is a variable"))
   sym->type = strdup("variable");
 else if (strstr(output, "is static"))
   sym->type = strdup("static");
 
 free(output);
 return sym;
}

/**
 * Get symbol at address
 */
mi_symbol *gmi_symbol_at_address(mi_h *h, void *addr)
{
 char cmd[512];
 char *output;
 mi_symbol *sym = NULL;
 
 if (!h || !addr)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "info symbol %p", addr);
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 /* Parse output like: "foo + 123 in section .text of /path/to/binary" */
 
 sym = mi_alloc_symbol();
 if (!sym) {
   free(output);
   return NULL;
 }
 
 sym->addr = addr;
 
 /* Extract symbol name (first word) */
 char *space = strchr(output, ' ');
 if (space) {
   int len = space - output;
   sym->name = strndup(output, len);
   if (!sym->name) {
     mi_free_symbol(sym);
     free(output);
     return NULL;
   }
 }
 
 free(output);
 return sym;
}

/**
 * Get line information
 */
mi_line_info *gmi_symbol_info_line(mi_h *h, const char *file, int line)
{
 char cmd[512];
 char *output;
 mi_line_info *info = NULL;
 
 if (!h || !file || line < 1)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "info line %s:%d", file, line);
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 /* Parse output like: "Line 42 of \"main.c\" starts at address 0x12345 <main+10> and ends at 0x12350 <main+21>." */
 
 info = mi_alloc_line_info();
 if (!info) {
   free(output);
   return NULL;
 }
 
 info->file = strdup(file);
 if (!info->file) {
   mi_free_line_info(info);
   free(output);
   return NULL;
 }
 info->line = line;
 
 char *start_str = strstr(output, "starts at address ");
 if (start_str) {
   start_str += strlen("starts at address ");
   char addr_buf[32];
   sscanf(start_str, "%31s", addr_buf);
   info->start_addr = parse_address(addr_buf);
 }
 
 char *end_str = strstr(output, "ends at ");
 if (end_str) {
   end_str += strlen("ends at ");
   char addr_buf[32];
   sscanf(end_str, "%31s", addr_buf);
   info->end_addr = parse_address(addr_buf);
 }
 
 free(output);
 return info;
}

/**
 * List functions matching regexp
 */
mi_function *gmi_symbol_list_functions(mi_h *h, const char *regexp)
{
 char cmd[512];
 char *output;
 mi_function *list = NULL, *current = NULL;
 
 if (!h)
   return NULL;
 
 if (regexp)
   snprintf(cmd, sizeof(cmd), "info functions %s", regexp);
 else
   snprintf(cmd, sizeof(cmd), "info functions");
 
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 /* Parse output - each function is on a line like:
  * "0x12345 main(int, char**) at main.c:10"
  * or "File main.c:"
  * or "int main(int, char**);"
  */
 
 char *line = strtok(output, "\n");
 char current_file[512] = "";
 
 while (line) {
   /* Skip empty lines and headers */
   while (*line == ' ' || *line == '\t') line++;
   
   if (strncmp(line, "File ", 5) == 0) {
     /* File header */
     char *file_start = line + 5;
     char *colon = strchr(file_start, ':');
     if (colon) {
       strncpy(current_file, file_start, colon - file_start);
       current_file[colon - file_start] = '\0';
     }
   } else if (line[0] == '0' && line[1] == 'x') {
     /* Function with address */
     mi_function *func = mi_alloc_function();
     if (!func)
       continue;
     
     /* Parse address */
     char addr_buf[32];
     sscanf(line, "%31s", addr_buf);
     func->addr = parse_address(addr_buf);
     
     /* Find function name after address */
     char *name_start = strchr(line, ' ');
     if (name_start) {
       name_start++;
       while (*name_start == ' ') name_start++;
       
       /* Extract name (up to '(' or ' ') */
       char *name_end = strpbrk(name_start, "( ");
       if (name_end) {
         func->name = strndup(name_start, name_end - name_start);
       }
       
       /* Extract file and line if present */
       char *at = strstr(name_start, " at ");
       if (at) {
         at += 4;
         char *colon = strchr(at, ':');
         if (colon) {
           func->file = strndup(at, colon - at);
           func->line = atoi(colon + 1);
         }
       } else if (current_file[0]) {
         func->file = strdup(current_file);
       }
     }
     
     /* Add to list */
     if (!list) {
       list = func;
       current = func;
     } else {
       current->next = func;
       current = func;
     }
   }
   
   line = strtok(NULL, "\n");
 }
 
 free(output);
 return list;
}

/**
 * Get type information using ptype
 */
mi_type_info *gmi_symbol_ptype(mi_h *h, const char *type_name)
{
 char cmd[512];
 char *output;
 mi_type_info *info = NULL;
 
 if (!h || !type_name)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "ptype %s", type_name);
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 info = mi_alloc_type_info();
 if (!info) {
   free(output);
   return NULL;
 }
 
 info->name = strdup(type_name);
 if (!info->name) {
   free(output);
   mi_free_type_info(info);
   return NULL;
 }
 
 /* Determine kind from output */
 if (strstr(output, "type = struct"))
   info->kind = strdup("struct");
 else if (strstr(output, "type = union"))
   info->kind = strdup("union");
 else if (strstr(output, "type = enum"))
   info->kind = strdup("enum");
 else if (strstr(output, "type = class"))
   info->kind = strdup("class");
 else
   info->kind = strdup("other");
 
 /* Store full type definition */
 info->members = output; /* Transfer ownership */
 
 return info;
}

/**
 * List variables matching regexp
 */
mi_variable *gmi_symbol_list_variables(mi_h *h, const char *regexp)
{
 char cmd[512];
 char *output;
 mi_variable *list = NULL, *current = NULL;
 
 if (!h)
   return NULL;
 
 if (regexp)
   snprintf(cmd, sizeof(cmd), "info variables %s", regexp);
 else
   snprintf(cmd, sizeof(cmd), "info variables");
 
 output = send_cli_command(h, cmd);
 
 if (!output)
   return NULL;
 
 /* Parse similar to functions */
 char *line = strtok(output, "\n");
 char current_file[512] = "";
 
 while (line) {
   while (*line == ' ' || *line == '\t') line++;
   
   if (strncmp(line, "File ", 5) == 0) {
     char *file_start = line + 5;
     char *colon = strchr(file_start, ':');
     if (colon) {
       strncpy(current_file, file_start, colon - file_start);
       current_file[colon - file_start] = '\0';
     }
   } else if (line[0] != '\0' && line[0] != 'N') {
     /* Variable line */
     mi_variable *var = mi_alloc_variable();
     if (!var)
       continue;
     
     /* Try to extract type and name */
     char *semicolon = strchr(line, ';');
     if (semicolon) {
       char *name_start = semicolon - 1;
       while (name_start > line && *name_start != ' ' && *name_start != '*') 
         name_start--;
       if (*name_start == ' ' || *name_start == '*')
         name_start++;
       
       var->name = strndup(name_start, semicolon - name_start);
       var->type = strndup(line, name_start - line);
       
       if (current_file[0])
         var->file = strdup(current_file);
       
       if (strstr(line, "static"))
         var->is_static = 1;
       
       /* Add to list */
       if (!list) {
         list = var;
         current = var;
       } else {
         current->next = var;
         current = var;
       }
     } else {
       mi_free_variable(var);
     }
   }
   
   line = strtok(NULL, "\n");
 }
 
 free(output);
 return list;
}

/* Allocation functions */
mi_symbol *mi_alloc_symbol(void) {
 return (mi_symbol *)mi_calloc1(sizeof(mi_symbol));
}

mi_line_info *mi_alloc_line_info(void) {
 return (mi_line_info *)mi_calloc1(sizeof(mi_line_info));
}

mi_function *mi_alloc_function(void) {
 return (mi_function *)mi_calloc1(sizeof(mi_function));
}

mi_type_info *mi_alloc_type_info(void) {
 return (mi_type_info *)mi_calloc1(sizeof(mi_type_info));
}

mi_variable *mi_alloc_variable(void) {
 return (mi_variable *)mi_calloc1(sizeof(mi_variable));
}

/* Deallocation functions */
void mi_free_symbol(mi_symbol *s) {
 if (!s) return;
 free(s->name);
 free(s->type);
 free(s->file);
 free(s->linkage_name);
 mi_free_symbol(s->next);
 free(s);
}

void mi_free_line_info(mi_line_info *l) {
 if (!l) return;
 free(l->file);
 mi_free_line_info(l->next);
 free(l);
}

void mi_free_function(mi_function *f) {
 if (!f) return;
 free(f->name);
 free(f->file);
 free(f->return_type);
 free(f->signature);
 mi_free_function(f->next);
 free(f);
}

void mi_free_type_info(mi_type_info *t) {
 if (!t) return;
 free(t->name);
 free(t->kind);
 free(t->file);
 free(t->members);
 mi_free_type_info(t->next);
 free(t);
}

void mi_free_variable(mi_variable *v) {
 if (!v) return;
 free(v->name);
 free(v->type);
 free(v->file);
 mi_free_variable(v->next);
 free(v);
}
