#ifndef _bam_access_h
#define _bam_access_h

#include "dbg.h"
#include "htslib/sam.h"
#include "bigWig.h"

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

int process_bam_file(char *input_file, bw_func func, tmpstruct_t *tmp, int filter, char *ref);

int process_bam_region(char *input_file, bw_func func, tmpstruct_t *tmp, int filter, char *region, char *ref);

int process_bam_region_bases(char *input_file, bw_func func, tmpstruct_t *tmp, int filter, char *region, char *ref);

int get_no_of_SQ_lines(char *input_file);

int build_chromList_from_bam(chromList_t *chromList ,char *bam_loc);

int parse_SQ_line(char *line, char *name, int *length);


#endif