/**[txh]********************************************************************

  Copyright (c) 2026
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  Module: Extended Symbol Query
  Comments:
  Extended GDB/MI commands for symbol information using CLI commands
  and MI parsing where MI commands are not available.

***************************************************************************/

#ifndef MI_SYMBOL_QUERY_EXT_H
#define MI_SYMBOL_QUERY_EXT_H

#include "mi_gdb.h"

/* Symbol information structure */
typedef struct mi_symbol_struct
{
 char *name;
 char *type;
 void *addr;
 char *file;
 int line;
 char *linkage_name; /* Mangled name for C++ */
 
 struct mi_symbol_struct *next;
} mi_symbol;

/* Source line information */
typedef struct mi_line_info_struct
{
 char *file;
 int line;
 void *start_addr;
 void *end_addr;
 
 struct mi_line_info_struct *next;
} mi_line_info;

/* Function information */
typedef struct mi_function_struct
{
 char *name;
 char *file;
 int line;
 void *addr;
 char *return_type;
 char *signature;
 int is_static;
 
 struct mi_function_struct *next;
} mi_function;

/* Type information */
typedef struct mi_type_info_struct
{
 char *name;
 char *kind;  /* struct, union, enum, class, typedef */
 int size;
 char *file;
 int line;
 char *members; /* Formatted member list */
 
 struct mi_type_info_struct *next;
} mi_type_info;

/* Variable information */
typedef struct mi_variable_struct
{
 char *name;
 char *type;
 char *file;
 int line;
 int is_static;
 int is_global;
 void *addr;
 
 struct mi_variable_struct *next;
} mi_variable;

/* Symbol query functions - use CLI fallback when MI not available */

/**
 * Get address of a symbol
 * Equivalent to: info address <symbol>
 */
mi_symbol *gmi_symbol_info_address(mi_h *h, const char *symbol);

/**
 * Get source file information for a symbol
 * Equivalent to: info symbol <addr>
 */
mi_symbol *gmi_symbol_at_address(mi_h *h, void *addr);

/**
 * Get line information for file:line or address
 * Equivalent to: info line <spec>
 */
mi_line_info *gmi_symbol_info_line(mi_h *h, const char *file, int line);
mi_line_info *gmi_symbol_info_line_addr(mi_h *h, void *addr);

/**
 * Get function information
 * Equivalent to: info functions [regexp]
 */
mi_function *gmi_symbol_list_functions(mi_h *h, const char *regexp);

/**
 * Get function at specific location
 */
mi_function *gmi_symbol_info_function(mi_h *h, const char *name);
mi_function *gmi_symbol_info_function_at(mi_h *h, const char *file, int line);

/**
 * Get type information
 * Equivalent to: ptype <type_name> or whatis <expr>
 */
mi_type_info *gmi_symbol_ptype(mi_h *h, const char *type_name);
mi_type_info *gmi_symbol_whatis(mi_h *h, const char *expression);

/**
 * List all types
 * Equivalent to: info types [regexp]
 */
mi_type_info *gmi_symbol_list_types(mi_h *h, const char *regexp);

/**
 * List variables
 * Equivalent to: info variables [regexp]
 */
mi_variable *gmi_symbol_list_variables(mi_h *h, const char *regexp);

/**
 * Get variables in scope at current location
 */
mi_variable *gmi_symbol_list_variables_in_scope(mi_h *h);

/**
 * List source files
 */
char **gmi_symbol_list_source_files(mi_h *h, int *count);

/**
 * Get lines in a source file
 * Uses -symbol-list-lines if available (gdb 6.x+)
 */
mi_line_info *gmi_symbol_list_lines(mi_h *h, const char *file);

/**
 * Memory allocation and deallocation
 */
mi_symbol *mi_alloc_symbol(void);
mi_line_info *mi_alloc_line_info(void);
mi_function *mi_alloc_function(void);
mi_type_info *mi_alloc_type_info(void);
mi_variable *mi_alloc_variable(void);

void mi_free_symbol(mi_symbol *s);
void mi_free_line_info(mi_line_info *l);
void mi_free_function(mi_function *f);
void mi_free_type_info(mi_type_info *t);
void mi_free_variable(mi_variable *v);

#endif /* MI_SYMBOL_QUERY_EXT_H */
