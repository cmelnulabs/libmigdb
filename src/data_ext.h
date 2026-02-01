/**[txh]********************************************************************

  Copyright (c) 2026

  Module: Enhanced Data Operations
  Comments:
  Additional data manipulation commands beyond basic libmigdb:
  - Memory write operations
  - Source code listing
  - Enhanced memory reading with format options
  - Searching memory

***************************************************************************/

#ifndef MI_DATA_EXT_H
#define MI_DATA_EXT_H

#include "mi_gdb.h"

/* Source line structure */
typedef struct mi_source_line_struct
{
 int line_num;
 char *text;
 int has_breakpoint;
 int is_current;
 
 struct mi_source_line_struct *next;
} mi_source_line;

/**
 * Write to memory
 * @param h      GDB handle
 * @param addr   Target address (can be expression)
 * @param data   Data to write
 * @param size   Size in bytes
 * @return 1 on success, 0 on failure
 */
int gmi_data_write_memory(mi_h *h, const char *addr, const unsigned char *data, 
                          unsigned int size);

/**
 * Write value to memory
 * @param addr      Target address
 * @param value     Value expression to write
 * @return 1 on success, 0 on failure
 */
int gmi_data_write_value(mi_h *h, const char *addr, const char *value);

/**
 * List source code lines
 * @param file      Source file path
 * @param start     Start line number (0 for current location)
 * @param count     Number of lines to list
 * @return Linked list of source lines or NULL
 */
mi_source_line *gmi_source_list(mi_h *h, const char *file, int start, int count);

/**
 * List source code around a function
 */
mi_source_line *gmi_source_list_function(mi_h *h, const char *function);

/**
 * List source code around an address
 */
mi_source_line *gmi_source_list_address(mi_h *h, void *addr);

/**
 * Get current source location
 */
mi_source_line *gmi_source_current_line(mi_h *h);

/**
 * Find pattern in memory
 * @param start_addr  Start address
 * @param length      Number of bytes to search
 * @param pattern     Pattern to search for
 * @param pattern_len Length of pattern
 * @param max_results Maximum results to return (0 for all)
 * @return Array of addresses where pattern was found, NULL-terminated
 */
void **gmi_data_find_pattern(mi_h *h, void *start_addr, unsigned long length,
                             const unsigned char *pattern, unsigned int pattern_len,
                             int max_results);

/**
 * Allocate and deallocate
 */
mi_source_line *mi_alloc_source_line(void);
void mi_free_source_line(mi_source_line *line);

#endif /* MI_DATA_EXT_H */
