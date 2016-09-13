#ifndef _utils_h
#define _utils_h

#include <stdint.h>
#include "dbg.h"

void print_version (int exit_code);

int line_count (char *file_path);

int check_exist(char *fname);

int parse_file_name( char *dir, char *fname, const char *file);

int parseRegionString(char *region, char *contig, uint32_t *start, uint32_t *stop);

#endif