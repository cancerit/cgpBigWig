/**   LICENSE
* Copyright (c) 2016-2018 Genome Research Ltd.
*
* Author: Cancer Genome Project cgphelp@sanger.ac.uk
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

chromList_t *build_chromList_from_bam_limit(char *bam_loc, khash_t(str) *contigs_h){
  chromList_t *chromList = NULL;
  htsFile *bam =  NULL;
  char *line = NULL;
  char *tmp = NULL;
  bam_hdr_t *header = NULL;
  char *ptr;

  chromList = malloc(sizeof(chromList_t));
  check_mem(chromList);
	int size_of_contigs = kh_size(contigs_h);
  chromList->nKeys = size_of_contigs;
  chromList->chrom = malloc(size_of_contigs*sizeof(char *));
  check_mem(chromList->chrom);
  chromList->len = malloc(size_of_contigs*sizeof(uint32_t));
  check_mem(chromList->len);

  bam = hts_open(bam_loc, "r");
  check(bam != 0,"Bam file %s failed to open to read header.",bam_loc);
  header = sam_hdr_read(bam);
  char *head_txt = header->text;
  khint_t k;

  line = strtok(head_txt,"\n");
  int i = 0;
	while(line != NULL){

		//Check for a read group line
		if(strncmp(line,"@SQ",3)==0){

      int is_missing = 0;
      //Iterate through each tag within the SQ line
      char *tag = strtok_r(line,"\t",&ptr);
      while(tag != NULL){

        int chk=0;
        tmp = malloc(sizeof(char) * 2048);
        check_mem(tmp);
        chk = sscanf(tag,"SN:%[^\t\n]",tmp);
        if(chk>0){

          //Check if the sequence name is one that's required
          k = kh_get(str, contigs_h, tmp);
          is_missing = (k == kh_end(contigs_h));

          //If this is an unrequired contig name
          if(is_missing) break;

          //Check if this sequence name is included in the contig list.
          chromList->chrom[i] = malloc(sizeof(char) * (strlen(tmp)+1));
          check_mem(chromList->chrom[i]);
          chromList->chrom[i] = strcpy(chromList->chrom[i],tmp);
          check(strcmp(chromList->chrom[i],tmp)==0,"Error copying %s to chromlist",tmp);

        }//End of check for sequence name tag
        //Check for sequence length tag
        uint32_t tmpint = 0;
        chk = sscanf(tag,"LN:%" SCNu32 "",&tmpint);
        if(chk>0 && is_missing==0){
          chromList->len[i] = tmpint;
          i++;
        }//End of check for sequence length tag

        tag = strtok_r(NULL,"\t",&ptr);
      }//End of iterating through each tag in the line

    }//End of if this is an SQ line
    line = strtok(NULL,"\n");
  }//End of iteration through headers

  free(line);
  bam_hdr_destroy(header);
  hts_close(bam);
  return chromList;
error:
  if(line) free(line);
  if(bam) hts_close(bam);
  if(header) bam_hdr_destroy(header);
  if(tmp) free (tmp);
  return NULL;
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

int process_bam_file(char *input_file, bw_func pileup_func, tmpstruct_t *tmp, int filter, int filter_inc, char *reference){

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
  int n_plp, tid, pos;
  const bam_pileup1_t *plp;
  int chck;
  while((reto = sam_read1(tmp->in, tmp->head, b)) >= 0){
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    //If the filter is not removing proper pairs
    //Assume that we have paried end data and check the orientations
    if((BAM_FPROPER_PAIR & filter_inc) > 0){
        if(((b->core.flag & BAM_FMREVERSE) && (b->core.flag & BAM_FREVERSE))) continue;
        if ((!(b->core.flag & BAM_FMREVERSE) && !(b->core.flag & BAM_FREVERSE))) continue;
    }
    int ret;
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
      chck = pileup_func(tid, pos, n_plp, plp, tmp);
      check(chck==0,"Error running pileup function.");
    }
  }
  bam_plp_push(buf,0);
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    chck = pileup_func(tid, pos, n_plp, plp, tmp);
    check(chck==0,"Error running pileup function.");
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

int process_bam_region_bases(char *input_file, bw_func_reg perbase_pileup_func, tmpstruct_t *perbase, int filter, 
                                                                        int filter_inc, char *region, char *reference){
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

  uint32_t last_pos = 0;
  uint32_t start;
  uint32_t stop;
  char *contig = parseRegionString(region, &start, &stop);
  int x=0;
  for(x=0;x<4;x++){
    perbase[x].idx = idx;
    perbase[x].in = file;
    perbase[x].head = head;
    if(start>1){
     perbase[x].lstart=start-1;
     perbase[x].lpos=start-1;
     last_pos = start-1;
    }
  }
  iter = sam_itr_querys(idx, head, region);
  int result;
  const bam_pileup1_t *plp;
  int ret, n_plp, tid, pos, chck;
  while ((result = sam_itr_next(file, iter, b)) >= 0) {
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    //If the filter is not removing proper pairs
    //Assume that we have paried end data and check the orientations
    if((BAM_FPROPER_PAIR & filter_inc) > 0){
        if(((b->core.flag & BAM_FMREVERSE) && (b->core.flag & BAM_FREVERSE))) continue;
        if ((!(b->core.flag & BAM_FMREVERSE) && !(b->core.flag & BAM_FREVERSE))) continue;
    }
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
      if(pos<start-1) continue;
      if(pos > last_pos+1){
        int x=0;
        for(x=0;x<4;x++){
          chck = perbase_pileup_func(perbase[x].ltid, (last_pos+1), 0, NULL, &perbase[x], start);
          check(chck==0,"Error running per base pileup.");
        }
      }
      int x=0;
      for(x=0;x<4;x++){
        chck = perbase_pileup_func(tid, pos, n_plp, plp, &perbase[x],start);
        check(chck==0,"Error running per base pileup.");
      }
      last_pos = pos;
      if(pos==stop) break;
    }
  }

  check(result >= -1, "Error detected (%d) when trying to iterate through region.",result);

  bam_plp_push(buf,0);
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    if(pos<start-1) continue;
    if(pos > last_pos+1){
      int x=0;
      for(x=0;x<4;x++){
        chck = perbase_pileup_func(perbase[x].ltid, (last_pos+1), 0, NULL, &perbase[x], start);
        check(chck==0,"Error running per base pileup.");
      }
    }
    int x=0;
    for(x=0;x<4;x++){
      chck = perbase_pileup_func(tid, pos, n_plp, plp, &perbase[x],start);
      check(chck==0,"Error running per base pileup.");
    }
    last_pos = pos;
    if(pos==stop) break;
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

int process_bam_region(char *input_file, bw_func_reg pileup_func, tmpstruct_t *tmp, int filter, int filter_inc, 
                                                                                        char *region, char *reference){

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
  uint32_t last_pos = 0;
  uint32_t reg_sta;
  uint32_t reg_sto;
  char *contig = parseRegionString(region, &reg_sta, &reg_sto);
  if(reg_sta>1){
   tmp->lstart=reg_sta-1;
   tmp->lpos=reg_sta-1;
   last_pos = reg_sta-1;
  }
  const bam_pileup1_t *plp;
  int ret, n_plp, tid, pos, chck;
  while ((result = sam_itr_next(tmp->in, iter, b)) >= 0) {
    if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
    //If the filter is not removing proper pairs
    //Assume that we have paried end data and check the orientations
    if((BAM_FPROPER_PAIR & filter_inc) > 0){
        if(((b->core.flag & BAM_FMREVERSE) && (b->core.flag & BAM_FREVERSE))) continue;
        if ((!(b->core.flag & BAM_FMREVERSE) && !(b->core.flag & BAM_FREVERSE))) continue;
    }
    ret = bam_plp_push(buf, b);
    if (ret < 0) break;
    while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != NULL){
      if(pos<reg_sta-1) continue;
      if(pos > last_pos+1){// && tmp->inczero == 1){
        //This is a region of zero coverage then...
        chck = pileup_func(tmp->ltid, (last_pos+1), 0, NULL, tmp, reg_sta); // Pileup call to set next cvg to zero
        check(chck==0,"Error running per base pileup.");
      }
      chck = pileup_func(tid, pos, n_plp, plp, tmp, reg_sta);
      check(chck==0,"Error running per base pileup.");
      last_pos = pos;
      if(pos==reg_sto) break;
    }

  }

  check(result >= -1, "Error detected (%d) when trying to iterate through region.",result);
  
  bam_plp_push(buf,0);

  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != NULL){
    if(pos<reg_sta-1) continue;
    if(pos > last_pos+1){// && tmp->inczero == 1){
      //This is a region of zero coverage then...
      chck = pileup_func(tmp->ltid, (last_pos+1), 0, NULL, tmp, reg_sta); // Pileup call to set next cvg to zero
      check(chck==0,"Error running per base pileup.");
    }
    chck = pileup_func(tid, pos, n_plp, plp, tmp, reg_sta);
    check(chck==0,"Error running per base pileup.");
    last_pos = pos;
    if(pos==reg_sto) break;
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
