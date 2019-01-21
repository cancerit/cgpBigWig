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
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "bam_access.h"
#include "utils.h"

static char *input_file = NULL;
static char *output_file = NULL;
static char *region = NULL;
static int filter = 0;
uint8_t is_overlap = 0;

KHASH_MAP_INIT_STR(strh,uint8_t)

void print_usage (int exit_code){

	printf ("Usage: bam2bedgraph -i input.[cr|b]am -o file [-r region] [-h] [-v]\n\n");
	printf ("Create a BEDGraph file of genomic coverage. BAM file must be sorted.\n");
  printf ("-i --input     Path to bam/cram input file. [default: stdin]\n");
  printf ("-o --output    File path for output. [default: stdout]\n\n");
	printf ("Optional:\n");
	printf ("-r --region    Region in bam to access.\n");
	printf ("-f --filter    Ignore reads with the filter flags [int].\n");
  printf("                N.B. if properly paired reads are not filtered out, bam2bedgraph will assume paired-end\n");
  printf("                and filter any proper-pair flagged reads not in F/R orientation.");
	printf ("-a --overlap   Use overlapping read check.\n\n");
	printf ("Other:\n");
	printf ("-h --help      Display this usage information.\n");
	printf ("-v --version   Prints the version number.\n\n");
  exit(exit_code);
}

void options(int argc, char *argv[]){
	const struct option long_opts[] =
	  {
             	{"version", no_argument, 0, 'v'},
             	{"help",no_argument,0,'h'},
              {"input",required_argument,0,'i'},
              {"region",required_argument,0,'r'},
              {"filter",required_argument,0,'f'},
              {"output",required_argument,0,'o'},
							{"overlap", no_argument, 0, 'a'},
              {"rna",no_argument,0, 'a'},
              { NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "i:o:r:f:avh", long_opts, &index)) != -1){
   	switch(iarg){
   		case 'i':
        input_file = optarg;
        break;

   		case 'o':
				output_file = optarg;
   			break;

   		case 'r':
   		  region = optarg;
   		  break;

   		case 'f':
   		  if(sscanf(optarg, "%i", &filter) != 1){
      		printf("Error parsing -f argument '%s'. Should be an integer > 0",optarg);
      		print_usage(1);
      	}
      	break;

			case 'a':
				is_overlap = 1;
				break;

   		case 'h':
        print_usage(0);
        break;

      case 'v':
        print_version(0);
        break;

			case '?':
        print_usage (1);
        break;

      default:
      	print_usage (1);

   	}; // End of args switch statement

   }//End of iteration through options

   //Do some checking to ensure required arguments were passed and are accessible files
   if (input_file==NULL || strcmp(input_file,"/dev/stdin")==0) {
    input_file = "-";   // htslib recognises this as a special case
   }
   if (strcmp(input_file,"-") != 0) {
     if(check_exist(input_file) != 1){
   	  printf("Input file (-i) %s does not exist.\n",input_file);
   	  print_usage(1);
     }
   }
   if (output_file==NULL || strcmp(output_file,"/dev/stdout")==0) {
    output_file = "-";   // we recognise this as a special case
   }

   return;
}

// callback for bam_plbuf_init()
static int pileup_func(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos          = (int)position;
  int coverage     = n;
  int i;
  for (i=0;i<n;i++)
    if (pl[i].is_del) coverage--;

  if (tmp->ltid != tid || tmp->lcoverage != coverage || pos > tmp->lpos+1) {
    if (tmp->lpos > 0 && tmp->lcoverage > 0)
      fprintf(tmp->out,"%s\t%d\t%d\t%d\n", tmp->head->target_name[tmp->ltid], tmp->lstart,tmp->lpos+1, tmp->lcoverage);
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  tmp->lpos = pos;
  return 0;
}

// callback for bam_plbuf_init()
static int pileup_func_overlap(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos          = (int)position;
  int coverage     = n;
  int i;
	khash_t(strh) *h;
	khiter_t k;
	h = kh_init(strh);
  for (i=0;i<n;i++){ // Iterate through each pileup object
    if (pl[i].is_del){
			coverage--;
			continue;
		}
		int absent;
		//Testing overlapping reads
		k = kh_put(strh, h, bam_get_qname(pl[i].b), &absent);
		uint8_t cbase = bam_seqi(bam_get_seq(pl[i].b),pl[i].qpos);
		uint8_t pre_b;
		if(!absent){ //Read already processed to get base processed (we only increment if base is different between overlapping read pairs)
			k = kh_get(strh, h, bam_get_qname(pl[i].b));
			pre_b = kh_val(h,k);
		}else{
			//Add the value to the hash
			kh_value(h, k) = cbase;
		}
		if(!absent && pre_b == cbase) coverage--; //Remove one from the total coverage if this is an overlap site
	}

  if (tmp->ltid != tid || tmp->lcoverage != coverage || pos > tmp->lpos+1) {
    if (tmp->lpos > 0 && tmp->lcoverage > 0)
      fprintf(tmp->out,"%s\t%d\t%d\t%d\n", tmp->head->target_name[tmp->ltid], tmp->lstart,tmp->lpos+1, tmp->lcoverage);
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  tmp->lpos = pos;
	kh_destroy(strh, h);
  return 0;
}

int main(int argc, char *argv[]){
	options(argc, argv);
  tmpstruct_t tmp;
  FILE *out = NULL;
	if (strcmp(output_file,"-")==0) {
    out = stdout;
  } else {
    out = fopen(output_file,"w");
  }
  check(out!=NULL,"Failed to open output file for %s writing.",output_file);
  tmp.out = out;
  int check = 0;
	bw_func func = &pileup_func;
	bw_func_reg func_reg = &pileup_func;
	if(is_overlap == 1){
		func = &pileup_func_overlap;
		func_reg = &pileup_func_overlap;
	}
	if(region == NULL){
	  check = process_bam_file(input_file, func, &tmp, filter, NULL);
	  check(check==1,"Error parsing bam file.");
	}else{
		check = process_bam_region(input_file, func_reg, &tmp, filter, region, NULL);
    check(check==1,"Error parsing bam region.");
	}
  fprintf(out,"%s\t%d\t%d\t%d\n", tmp.head->target_name[tmp.ltid], tmp.lstart,tmp.lpos+1, tmp.lcoverage);

  fflush(out);
  fclose(out);
  if(tmp.idx) hts_idx_destroy(tmp.idx);
	if(tmp.in) hts_close(tmp.in);
	if(tmp.head) bam_hdr_destroy(tmp.head);
	return 0;

error:
  if(out) fclose(out);
  return -1;
}
