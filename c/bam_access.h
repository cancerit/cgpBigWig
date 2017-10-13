/**   LICENSE
* Copyright (c) 2016 Genome Research Ltd.
*
* Author: Cancer Genome Project cgpit@sanger.ac.uk
*
* This file is part of cgpBigWig.
*
* cgpBigWig is free software: you can redistribute it and/or modify it under
* the terms of the GNU Affero General Public License as published by the Free
* Software Foundation; either version 3 of the License, or (at your option) any
* later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
* details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*    1. The usage of a range of years within a copyright statement contained within
*    this distribution should be interpreted as being equivalent to a list of years
*    including the first and last year specified and all consecutive years between
*    them. For example, a copyright statement that reads ‘Copyright (c) 2005, 2007-
*    2009, 2011-2012’ should be interpreted as being identical to a statement that
*    reads ‘Copyright (c) 2005, 2007, 2008, 2009, 2011, 2012’ and a copyright
*    statement that reads ‘Copyright (c) 2005-2012’ should be interpreted as being
*    identical to a statement that reads ‘Copyright (c) 2005, 2006, 2007, 2008,
*    2009, 2010, 2011, 2012’."
*
*/

#ifndef _bam_access_h
#define _bam_access_h

#include "dbg.h"
#include "htslib/sam.h"
#include "khash.h"
#include "bigWig.h"

KHASH_SET_INIT_STR(str)

typedef struct {
  uint32_t ltid;
  int      lstart,lcoverage,lpos,beg,end;
  htsFile *in;
  hts_idx_t *idx;
	bam_hdr_t *head;
	FILE *out;
	bigWigFile_t *bwout;
	float lbaseprop;
	uint8_t base_bit;
	uint8_t inczero;
	uint32_t reg_start;
	uint32_t reg_stop;
} tmpstruct_t;

typedef int (*bw_func)(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data);

typedef int (*bw_func_reg)(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data, uint32_t reg_start);

int process_bam_file(char *input_file, bw_func func, tmpstruct_t *tmp, int filter, char *ref);

int process_bam_region(char *input_file, bw_func_reg func, tmpstruct_t *tmp, int filter, char *region, char *ref);

int process_bam_region_bases(char *input_file, bw_func_reg func, tmpstruct_t *tmp, int filter, char *region, char *ref);

int get_no_of_SQ_lines(char *input_file);

int build_chromList_from_bam(chromList_t *chromList, char *bam_loc);

chromList_t *build_chromList_from_bam_limit(char *bam_loc, khash_t(str) *contigs_h);

int parse_SQ_line(char *line, char *name, int *length);


#endif
