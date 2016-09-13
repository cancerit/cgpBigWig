#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "bam_access.h"

int build_chromList_from_bam(chromList_t *chromList ,char *bam_loc){
  htsFile *bam =  NULL;
  char *line = NULL;
  char *tmp = NULL;
  bam_hdr_t *header = NULL;
  char *ptr;
	bam = hts_open(bam_loc, "r");
	check(bam != 0,"Bam file %s failed to open to read header.",bam_loc);
	header = sam_hdr_read(bam);
	char *head_txt = header->text;
  line = strtok(head_txt,"\n");
  int i = 0;
	while(line != NULL){
		//Check for a read group line
		if(strncmp(line,"@SQ",3)==0){
      char *tag = strtok_r(line,"\t",&ptr);
      while(tag != NULL){
        int chk=0;
        tmp = malloc(sizeof(char) * 512);
        check_mem(tmp);
        chk = sscanf(tag,"SN:%[^\t\n]",tmp);
        if(chk>0){
          chromList->chrom[i] = malloc(sizeof(char) * (strlen(tmp)+1));
          check_mem(chromList->chrom[i]);
          strcpy(chromList->chrom[i],tmp);
          tag = strtok_r(NULL,"\t",&ptr);
          free(tmp);
          continue;
        }
        free(tmp);
        uint32_t tmpint = 0;
        chk = sscanf(tag,"LN:%" SCNu32 "",&tmpint);
        if(chk>0){
          chromList->len[i] = tmpint;
          tag = strtok_r(NULL,"\t",&ptr);
          continue;
        }
        tag = strtok_r(NULL,"\t",&ptr);
      }//End of SQ line tags
      i++;
		}//End of if this is an SQ line
		line = strtok(NULL,"\n");
	}
	free(line);
	bam_hdr_destroy(header);
	hts_close(bam);
  return 1;
error:
  if(line) free(line);
	if(bam) hts_close(bam);
	if(header) bam_hdr_destroy(header);
	if(tmp) free (tmp);
  return -1;
}

int parse_SQ_line(char *line, char *name, int *length){
  char *tag = NULL;
  char *tmp = NULL;
  char *ptr = NULL;
  int nom = 0;
  int len = 0;
  tag = strtok_r(line,"\t",&ptr);
  while(tag != NULL){
    int chk=0;
    tmp = malloc(sizeof(char) * 512);
    check_mem(tmp);
    chk = sscanf(tag,"SN:%[^\t\n]",tmp);
    if(chk>0){
      strcpy(name,tmp);
      tag = strtok_r(NULL,"\t",&ptr);
      nom = 1;
      continue;
    }
    chk = sscanf(tag,"LN:%d",length);
    if(chk>0){
      len = 1;
      tag = strtok_r(NULL,"\t",&ptr);
      continue;
    }
    tag = strtok_r(NULL,"\t",&ptr);
  }
  if(tmp) free(tmp);
  if(tag) free(tag);

  if(nom && len) {
    return 1;
  }else{
    return -1;
  }
error:
  if(tmp) free(tmp);
  if(tag) free(tag);
  return -1;
}

int get_no_of_SQ_lines(char *bam_loc){
  htsFile *bam =  NULL;
  char *line = NULL;
  bam_hdr_t *header = NULL;
	bam = hts_open(bam_loc, "r");
	check(bam != 0,"Bam file %s failed to open to read header.",bam_loc);
	header = sam_hdr_read(bam);
	char *head_txt = header->text;
  line = strtok(head_txt,"\n");
  int count = 0;
	while(line != NULL){
		//Check for a read group line
		if(strncmp(line,"@SQ",3)==0){
		  count++;
		}//End of if this is an SQ line
		line = strtok(NULL,"\n");
	}
	free(line);
	hts_close(bam);
	bam_hdr_destroy(header);
  return count;
error:
  if(line) free(line);
	if(bam) hts_close(bam);
	if(header) bam_hdr_destroy(header);
  return -1;
}

int process_bam_file(char *input_file, bw_func pileup_func, tmpstruct_t *tmp, int filter, char *reference){

  bam_plp_t buf = NULL;
	bam1_t *b = NULL;
  hts_itr_t *iter = NULL;
  tmp->beg = 0; tmp->end = 0x7fffffff;
	tmp->lstart    = 0;
	tmp->lcoverage = 0;
	tmp->ltid      = 0;
	tmp->lpos      = 0;

  tmp->in = hts_open(input_file, "r");
  check(tmp->in!=0,"Fail to open [CR|B]AM file %s\n", input_file);
  if(reference){
    int check = hts_set_fai_filename(tmp->in, reference);
    check(check==0,"Error setting reference %s for hts file %s.",reference,input_file);
  }
  tmp->head = sam_hdr_read(tmp->in);
  buf = bam_plp_init(0,0);
  //Iterate through each read in bam file.
  b = bam_init1();
  tmp->idx=NULL;
  int reto;
  while((reto = sam_read1(tmp->in, tmp->head, b)) >= 0){
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    int ret, n_plp, tid, pos;
    const bam_pileup1_t *plp;
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0)
        pileup_func(tid, pos, n_plp, plp, tmp);
  }
  bam_plp_push(buf,0);

  //Check we've written everything...
	int n_plp, tid, pos;
  const bam_pileup1_t *plp;
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    pileup_func(tid, pos, n_plp, plp, tmp);
  }
  bam_plp_destroy(buf);
  bam_destroy1(b);

  if(iter) sam_itr_destroy(iter);
  return 1;

error:
  if(iter) sam_itr_destroy(iter);
  if(tmp->idx) hts_idx_destroy(tmp->idx);
	if(tmp->in) hts_close(tmp->in);
  if(tmp->head) bam_hdr_destroy(tmp->head);
  if(tmp->in) hts_close(tmp->in);
  return -1;
}

int process_bam_region_bases(char *input_file, bw_func perbase_pileup_func, tmpstruct_t *perbase, int filter, char *region, char *reference){
  bam_plp_t buf = NULL;
	bam1_t *b = NULL;
  hts_itr_t *iter = NULL;
  htsFile *file = NULL;
  hts_idx_t *idx = NULL;
	bam_hdr_t *head = NULL;
  file= hts_open(input_file, "r");
  check(file!=0,"Fail to open [CR|B]AM file %s\n", input_file);
  if(reference){
    int check = hts_set_fai_filename(file, reference);
    check(check==0,"Error setting reference %s for hts file %s.",reference,input_file);
  }
  head = sam_hdr_read(file);
  buf = bam_plp_init(0,0);
  //Iterate through each read in bam file.
  b = bam_init1();
  idx = sam_index_load(file,input_file);
  check(idx != 0, "Fail to open [CR|B]AM index for file %s\n", input_file);
  int x=0;
  for(x=0;x<4;x++){
    perbase[x].idx = idx;
    perbase[x].in = file;
    perbase[x].head = head;
  }
  uint32_t start;
  uint32_t stop;
  char *contig = malloc(sizeof(char) * 2048);
  parseRegionString(region, contig, &start, &stop);
  iter = sam_itr_querys(idx, head, region);
  int result;
  while ((result = sam_itr_next(file, iter, b)) >= 0) {
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    int ret, n_plp, tid, pos;
    const bam_pileup1_t *plp;
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
      if(pos >= start-1 && pos <= stop){
        int x=0;
        for(x=0;x<4;x++){
          perbase_pileup_func(tid, pos, n_plp, plp, &perbase[x]);
        }
      }
    }
  }
  bam_plp_push(buf,0);
  //Check we've written everything...
	int n_plp, tid, pos;
  const bam_pileup1_t *plp;
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    if(pos >= start-1 && pos <= stop){
      int x=0;
      for(x=0;x<4;x++){
        perbase_pileup_func(tid, pos, n_plp, plp, &perbase[x]);
      }
    }
  }
  bam_plp_destroy(buf);
  bam_destroy1(b);

  if(iter) sam_itr_destroy(iter);
  if(idx) hts_idx_destroy(idx);
	if(file) hts_close(file);
  return 1;
error:
  if(iter) sam_itr_destroy(iter);
  if(idx) hts_idx_destroy(idx);
	if(file) hts_close(file);
	if(head) bam_hdr_destroy(head);
  return -1;
}

int process_bam_region(char *input_file, bw_func pileup_func, tmpstruct_t *tmp, int filter, char *region, char *reference){

  bam_plp_t buf = NULL;
	bam1_t *b = NULL;
  hts_itr_t *iter = NULL;
  tmp->beg = 0; tmp->end = 0x7fffffff;
	tmp->lstart    = 0;
	tmp->lcoverage = 0;
	tmp->ltid      = 0;
	tmp->lpos      = 0;

  tmp->in = hts_open(input_file, "r");
  check(tmp->in!=0,"Fail to open [CR|B]AM file %s\n", input_file);
  if(reference){
    int check = hts_set_fai_filename(tmp->in, reference);
    check(check==0,"Error setting reference %s for hts file %s.",reference,input_file);
  }
  tmp->head = sam_hdr_read(tmp->in);
  buf = bam_plp_init(0,0);
  //Iterate through each read in bam file.
  b = bam_init1();

  tmp->idx = sam_index_load(tmp->in,input_file);
  check(tmp->idx != 0, "Fail to open [CR|B]AM index for file %s\n", input_file);
  iter = sam_itr_querys(tmp->idx, tmp->head, region);
  int result;
  while ((result = sam_itr_next(tmp->in, iter, b)) >= 0) {
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    int ret, n_plp, tid, pos;
    const bam_pileup1_t *plp;
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
        pileup_func(tid, pos, n_plp, plp, tmp);
    }
  }
  bam_plp_push(buf,0);
  //Check we've written everything...
	int n_plp, tid, pos;
  const bam_pileup1_t *plp;
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    pileup_func(tid, pos, n_plp, plp, tmp);
  }
  bam_plp_destroy(buf);
  bam_destroy1(b);

  if(iter) sam_itr_destroy(iter);
  if(tmp->idx) hts_idx_destroy(tmp->idx);
	if(tmp->in) hts_close(tmp->in);
  return 1;
error:
  if(iter) sam_itr_destroy(iter);
  if(tmp->idx) hts_idx_destroy(tmp->idx);
	if(tmp->in) hts_close(tmp->in);
  if(tmp->head) bam_hdr_destroy(tmp->head);
  return -1;
}