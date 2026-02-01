/**[txh]********************************************************************

  Copyright (c) 2026

  Module: Enhanced Data Operations Implementation

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mi_gdb.h"
#include "data_ext.h"

int gmi_data_write_memory(mi_h *h, const char *addr, const unsigned char *data, 
                          unsigned int size)
{
 char cmd[1024];
 char hex_data[512];
 unsigned int i;
 
 if (!h || !addr || !data || size == 0)
   return 0;
 
 /* Convert data to hex string */
 if (size * 2 >= sizeof(hex_data))
   return 0;
 
 for (i = 0; i < size; i++) {
   snprintf(hex_data + i*2, 3, "%02x", data[i]);
 }
 hex_data[size * 2] = '\0';
 
 /* Use restore command or set command */
 snprintf(cmd, sizeof(cmd), "set {unsigned char[%u]}(%s) = {%s}", 
          size, addr, hex_data);
 
 mi_send(h, "-interpreter-exec console \"%s\"\n", cmd);
 
 return mi_res_simple_done(h);
}

int gmi_data_write_value(mi_h *h, const char *addr, const char *value)
{
 char cmd[1024];
 
 if (!h || !addr || !value)
   return 0;
 
 snprintf(cmd, sizeof(cmd), "set {long}(%s) = %s", addr, value);
 
 mi_send(h, "-interpreter-exec console \"%s\"\n", cmd);
 
 return mi_res_simple_done(h);
}

static char *send_cli_command(mi_h *h, const char *command)
{
 char *result = NULL;
 
 if (!h || !command)
   return NULL;
 
 h->catch_console = 1;
 h->catched_console = NULL;
 
 mi_send(h, "-interpreter-exec console \"%s\"\n", command);
 
 if (!mi_get_response(h)) {
   h->catch_console = 0;
   return NULL;
 }
 
 result = h->catched_console;
 h->catched_console = NULL;
 h->catch_console = 0;
 
 return result;
}

mi_source_line *gmi_source_list(mi_h *h, const char *file, int start, int count)
{
 char cmd[512];
 char *output;
 mi_source_line *list = NULL, *current = NULL;
 
 if (!h)
   return NULL;
 
 if (file && start > 0) {
   if (count > 0)
     snprintf(cmd, sizeof(cmd), "list %s:%d,%d", file, start, start + count - 1);
   else
     snprintf(cmd, sizeof(cmd), "list %s:%d", file, start);
 } else if (file) {
   snprintf(cmd, sizeof(cmd), "list %s:1", file);
 } else if (count > 0) {
   snprintf(cmd, sizeof(cmd), "list *$pc,%d", count);
 } else {
   snprintf(cmd, sizeof(cmd), "list");
 }
 
 output = send_cli_command(h, cmd);
 if (!output)
   return NULL;
 
 /* Parse output like:
  * "42    int x = 10;"
  * "43    return x;"
  */
 
 char *line = strtok(output, "\n");
 while (line) {
   mi_source_line *src_line = mi_alloc_source_line();
   if (!src_line) {
     mi_free_source_line(list);
     free(output);
     return NULL;
   }
   
   /* Parse line number */
   int line_num;
   if (sscanf(line, "%d", &line_num) == 1) {
     src_line->line_num = line_num;
     
     /* Find the actual source text (after line number and whitespace) */
     char *text_start = line;
     while (*text_start && (*text_start == ' ' || *text_start == '\t' || 
                           (*text_start >= '0' && *text_start <= '9')))
       text_start++;
     
     if (*text_start)
       src_line->text = strdup(text_start);
     
     /* Add to list */
     if (!list) {
       list = src_line;
       current = src_line;
     } else {
       current->next = src_line;
       current = src_line;
     }
   } else {
     mi_free_source_line(src_line);
   }
   
   line = strtok(NULL, "\n");
 }
 
 free(output);
 return list;
}

mi_source_line *gmi_source_list_function(mi_h *h, const char *function)
{
 char cmd[512];
 char *output;
 
 if (!h || !function)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "list %s", function);
 
 /* Similar parsing as above */
 return gmi_source_list(h, NULL, 0, 0);
}

mi_source_line *gmi_source_list_address(mi_h *h, void *addr)
{
 char cmd[512];
 
 if (!h || !addr)
   return NULL;
 
 snprintf(cmd, sizeof(cmd), "list *%p", addr);
 
 char *output = send_cli_command(h, cmd);
 if (!output)
   return NULL;
 
 free(output);
 
 return gmi_source_list(h, NULL, 0, 10);
}

mi_source_line *gmi_source_current_line(mi_h *h)
{
 mi_frames *frame;
 
 if (!h)
   return NULL;
 
 frame = gmi_stack_info_frame(h);
 if (!frame)
   return NULL;
 
 mi_source_line *line = gmi_source_list(h, frame->file, frame->line, 1);
 
 if (line)
   line->is_current = 1;
 
 mi_free_frames(frame);
 return line;
}

void **gmi_data_find_pattern(mi_h *h, void *start_addr, unsigned long length,
                             const unsigned char *pattern, unsigned int pattern_len,
                             int max_results)
{
 /* This would require reading memory and searching in the wrapper */
 /* For now, return NULL - would need to implement memory scanning */
 return NULL;
}

mi_source_line *mi_alloc_source_line(void)
{
 return (mi_source_line *)mi_calloc1(sizeof(mi_source_line));
}

void mi_free_source_line(mi_source_line *line)
{
 if (!line) return;
 free(line->text);
 mi_free_source_line(line->next);
 free(line);
}
