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

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <alloca.h>
#include <inttypes.h>
#include "bam_access.h"
#include "utils.h"

KHASH_MAP_INIT_STR(strh,uint8_t)

char *out_file = "output.bam.bw";
char *input_file = NULL;
char *region_store = NULL;
char **our_region_list = NULL;
char *reference = NULL;
uint32_t region_list_count = 0;
int is_regions_file = 0;
uint8_t is_base = 0;
int filter = 4;
int filterinc = 0;
char base = 0;
uint8_t is_overlap = 0;
int include_zeroes = 0;
uint32_t single = 1;
char *last_contig = "";
float scale = 1;
float scale_log10 = 0;

void print_usage (int exit_code){
	printf("Usage: bam2bw -i input.[b|cr]am -o output.bw\n");
	printf("bam2bw can be used to generate a bw file of coverage from a [cr|b]am file.\n\n");
	printf("-i  --input [file]                                Path to the input [b|cr]am file.\n");
	printf("-F  --filter [int]                                SAM flags to filter. [default: %d]\n",filter);
  printf("-f  --filter-include [int]                        SAM flags to include. [default: %d]\n",filterinc);
  printf("                                                  N.B. if properly paired reads are filtered for inclusion bam2bw will assume paired-end data\n");
  printf("                                                  and exclude any proper-pair flagged reads not in F/R orientation.");

	printf("-o  --outfile [file]                              Path to the output .bw file produced. [default:'%s']\n\n",out_file);
	printf("Optional: \n");
	printf("-S  --scale-log10 [float]                         A scale factor to multiply to output [default: %d]\n", scale_log10);
  printf("-c  --region [file]                               A samtools style region (contig:start-stop) or a bed file of regions over which to produce the bigwig file\n");
	printf("-z  --include-zeroes                              Include zero coverage regions as additional entries to the bw file\n");
	printf("-r  --reference [file]                            Path to reference genome.fa file (required for cram if ref_path cannot be resolved)\n");
	printf("-a  --overlap                                     Use overlapping read check\n\n");
  printf ("Other:\n");
	printf("-h  --help                                        Display this usage information.\n");
	printf("-v  --version                                     Prints the version number.\n\n");
  exit(exit_code);
}

int check_region_string(char *region){
  if(check_exist(region)==1){
    is_regions_file = 1;
    return 1;
  }else{
    //Parse this as a region
    int beg = 0;
    int end = 0;
    const char *res = hts_parse_reg(region, &beg, &end);
    if(res) return 1;
  }
  return -1;
}

int get_int_length(int input){
  return (input == 0 ? 1 : (int)(log10(input)+1));
}


static float _rescale( float cvg, float scale){
    cvg = cvg*scale;
//     fprintf(stderr,"[scale],%f\n",cvg);
//     for (i=0;i<n;i++) values[i] *= scale;
    return cvg;
}

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"input", required_argument, 0, 'i'},
             	{"filter", required_argument, 0, 'F'},
              {"filter-include", required_argument, 0, 'f'}, 
             	{"outfile",required_argument, 0, 'o'},
             	{"region",required_argument, 0, 'c'},
             	{"reference",required_argument, 0, 'r'},
             	{"include-zeroes",no_argument, 0, 'z'},
							{"overlap", no_argument, 0, 'a'},
             	{"help", no_argument, 0, 'h'},
              {"scale-log10", required_argument, 0, 'S'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "S:F:f:i:o:c:r:azhv",long_opts, &index)) != -1){
    switch(iarg){
      case 'F':
        if(sscanf(optarg, "%i", &filter) != 1){
      		fprintf(stderr,"Error parsing -F|--filter argument '%s'. Should be an integer > 0",optarg);
      		print_usage(1);
      	}
        break;
      case 'f':
        if(sscanf(optarg, "%i", &filterinc) != 1){
      		fprintf(stderr,"Error parsing -f|--filter-include argument '%s'. Should be an integer > 0",optarg);
      		print_usage(1);
      	}
        break;
      case 'S':
       if(sscanf(optarg, "%f", &scale_log10) != 1){
        fprintf(stderr,"Error parsing -S|--scale-log10 argument '%s'. Should be an float.",optarg);
          print_usage(1);
        }
       scale = pow(10., scale_log10);
       fprintf(stderr,"[scale_factor],%E\n",scale);
       break;       
   		case 'i':
				input_file = optarg;
				if(check_exist(input_file) != 1){
          fprintf(stderr,"Input bam file %s does not appear to exist.\n",input_file);
          print_usage(1);
        }
   			break;
   		case 'o':
				out_file = optarg;
   			break;
			case 'h':
				print_usage (0);
				break;
			case 'v':
				print_version (0);
				break;
			case 'c':
			  region_store = optarg;
			  //First check for a region format
			  int res = check_region_string(region_store);
        if(res<0){
          fprintf(stderr,"Region %s is not in correct format or not an existing bed file.\n",region_store);
          print_usage(1);
        }
			  break;
			case 'r':
			  reference = optarg;
			  break;
			case 'z':
			  include_zeroes  = 1;
			  break;
			case 'a':
				is_overlap = 1;
				break;
			case '?':
        print_usage (1);
        break;
      default:
      	print_usage (0);
   	}; // End of args switch statement

   }//End of iteration through options

  if(input_file==NULL){
    fprintf(stderr,"Required option -i|--input-bam not defined.\n");
    print_usage(1);
  }

  if(is_base && region_store==NULL){
    fprintf(stderr,"Option -r|--region must be used with the -b|--base option.\n");
    print_usage(1);
  }

  return;
}

// callback for bam_plbuf_init()
static int pileup_func(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data, uint32_t reg_start){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos          = (int)position;
  int coverage     = n;
  int i;
  for (i=0;i<n;i++)
    if (pl[i].is_del) coverage--;
  if((uint32_t)pos == reg_start-1){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  if (tmp->ltid != tid || tmp->lcoverage != coverage || pos > tmp->lpos+1) {
    if (tmp->inczero == 1 || tmp->lcoverage > 0 ){
      uint32_t start =  tmp->lstart;
      uint32_t stop = pos;
      float cvg = (float)tmp->lcoverage;
      cvg = _rescale(cvg,scale);
			if(tmp->lcoverage == 0 && tmp->ltid != tid-1 && tmp->ltid != tid){
				tmp->ltid = tid;
			}
      int chck = bwAddIntervals(tmp->bwout,
									&tmp->head->target_name[tmp->ltid],&start,&stop,&cvg,single);
      check(chck==0,"Error adding bw interval %s:%"PRIu32"-%"PRIu32" = %f . Error code: %d",tmp->head->target_name[tmp->ltid],start,stop,cvg,chck);
    }
    //if(tmp->inczero == 1 && tmp->ltid != tid && pos != tmp->head->target_len[tmp->ltid]){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  tmp->lpos = pos;
  return 0;
error:
  return 1;
}

// callback for bam_plbuf_init() for overlapping reads
static int pileup_func_overlap(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data, uint32_t reg_start){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos          = (int)position;
  int coverage     = n;
  int i;

	khash_t(strh) *h;
	khiter_t k;
	h = kh_init(strh);

  for (i=0;i<n;i++){
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

  if((uint32_t)pos == reg_start-1){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  if (tmp->ltid != tid || tmp->lcoverage != coverage || pos > tmp->lpos+1) {
    if (tmp->inczero == 1 || tmp->lcoverage > 0 ){
      uint32_t start =  tmp->lstart;
      uint32_t stop = pos;
      float cvg = (float)tmp->lcoverage;
      cvg = _rescale(cvg,scale);

			if(tmp->lcoverage == 0 && tmp->ltid != tid-1 && tmp->ltid != tid){
				tmp->ltid = tid;
			}
      int chck = bwAddIntervals(tmp->bwout,
									&tmp->head->target_name[tmp->ltid],&start,&stop,&cvg,single);
      check(chck==0,"Error adding bw interval %s:%"PRIu32"-%"PRIu32" = %f . Error code: %d",tmp->head->target_name[tmp->ltid],start,stop,cvg,chck);
    }
    //if(tmp->inczero == 1 && tmp->ltid != tid && pos != tmp->head->target_len[tmp->ltid]){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
  }
  tmp->lpos = pos;
	kh_destroy(strh, h);
  return 0;
error:
	kh_destroy(strh, h);
  return 1;
}

bigWigFile_t *initialise_bw_output(char *out_file, chromList_t *chromList){
  //Open output file
  bigWigFile_t *fp = bwOpen(out_file, NULL, "w");
  check(fp, "Error opening %s for writing.",out_file);
  //10 zoom levels
  int res = bwCreateHdr(fp, 10);
  check(!res,"Received error %d in bwCreateHeader for %s",res,out_file);
  //Create the chromosome lists
  fp->cl = bwCreateChromList(chromList->chrom, chromList->len, chromList->nKeys);
  check(fp->cl != NULL, "Error generating chromlist as bw for %s.",out_file);
  res = bwWriteHdr(fp);
  check(!res,"Error %d writing bw header for %s.",res,out_file);

  return fp;
error:
  return NULL;
}

uint32_t getContigLength(char *contig,chromList_t *chromList){
  int i=0;
  for(i=0;i<chromList->nKeys;i++){
    if(strcmp(contig,chromList->chrom[i])==0) return chromList->len[i];
  }
  return -1;
}

int main(int argc, char *argv[]){
	setup_options(argc, argv);
	tmpstruct_t tmp;
	int no_of_regions = 0;

	//setup hash to count unique contigs
	khash_t(str) *contigs_h = NULL;
	khint_t k;
	contigs_h = kh_init(str);

	int sq_lines = get_no_of_SQ_lines(input_file);
	FILE *fp_bed = NULL;

	//Build list of contigs for chrom list from the header of the bamfile
	htsFile *bam =  NULL;
	char *line_hd = NULL;
	bam = hts_open(input_file, "r");
	check(bam != 0,"Bam file %s failed to open to read header.",input_file);
	bam_hdr_t *header = sam_hdr_read(bam);
	char *head_txt = header->text;
	line_hd = strtok(head_txt,"\n");
	while(line_hd != NULL){
		//Check for a sequence line
		if(strncmp(line_hd,"@SQ",3)==0){
			char *contig = malloc(sizeof(char) * 1024);
			int end,chk;
			chk = parse_SQ_line(line_hd,contig,&end);
			check(chk==1,"Error parsing SQ line %s.\n",line_hd);
			//Attempt to add contig as key to hash
			int absent;
			k = kh_put(str, contigs_h, contig, &absent);
			if (absent) kh_key(contigs_h, k) = strdup(contig);
		}//End of if this is an SQ line
		line_hd = strtok(NULL,"\n");
	}
	bam_hdr_destroy(header);
	free(line_hd);
	hts_close(bam);

	if(region_store){
    if(is_regions_file==1){
      //If we have a bedfile or regions
      no_of_regions = line_count(region_store);
      check(no_of_regions>0,"Error counting entries in region file %s\n",region_store);
      our_region_list = calloc(no_of_regions, sizeof(char *));
      check_mem(region_list);
      int i=0;
      fp_bed = fopen(region_store,"r");
        char line[512];
        while(fgets(line,512,fp_bed)){
          char *contig = malloc(sizeof(char)*1024);
          check_mem(contig);
          int beg,end;
          int chk = sscanf(line,"%s\t%d\t%d",contig,&beg,&end);
          check(chk==3,"Error reading line '%s' from regions bed file.",line);
          char *region = malloc(sizeof(char) * (strlen(contig)+get_int_length(beg)+get_int_length(end))+3);
          check_mem(region);
          chk = sprintf(region,"%s:%d-%d",contig,beg+1,end); //+1 to beginning as this is bed
          check(chk==((strlen(contig)+get_int_length(beg)+get_int_length(end))+2),"Error creating region line from bed entry '%s'.",line);
          our_region_list[i] = region;
          i++;
        }
      fclose(fp_bed);
    }else{ // End of if this has a regions file as input
      //If we have a single region....
      no_of_regions = 1;
      our_region_list = calloc(no_of_regions, sizeof(char *));
      check_mem(region_list);
			char *contig = malloc(sizeof(char)*1024);
			check_mem(contig);
			int beg,end;

			const char *q;
			q = hts_parse_reg(region_store,&beg,&end);

			if (q) {
        char tmp_a[1024], *contig = tmp_a;
        if (q - region_store + 1 > 1024)
            if (!(contig = malloc(q - region_store + 1)))
                sentinel("Error allocating memory for region parsing.");
        strncpy(contig, region_store, q - region_store);
        contig[q - region_store] = 0;
			}else{
				// not parsable as a region, but possibly a sequence named "foo:a"
        beg = 0; end = INT_MAX;
			}
			free(contig);

      our_region_list[0] = region_store;
    }
	}else{
    //If not...
    //Build a list of regions from the header of the bamfile
    no_of_regions = sq_lines;
    our_region_list = calloc(no_of_regions, sizeof(char *));
		bam =  NULL;
    char *line = NULL;
    bam = hts_open(input_file, "r");
    check(bam != 0,"Bam file %s failed to open to read header.",input_file);
    bam_hdr_t *header = sam_hdr_read(bam);
    char *head_txt = header->text;
    line = strtok(head_txt,"\n");
    int i=0;
    while(line != NULL){
      //Check for a sequence line
      if(strncmp(line,"@SQ",3)==0){
        char *contig = malloc(sizeof(char) * 1024);
        int end,chk;
        chk = parse_SQ_line(line,contig,&end);
        check(chk==1,"Error parsing SQ line %s.\n",line);
        char *region = malloc(sizeof(char) * (strlen(contig)+get_int_length(end))+4);
        check_mem(region);
        chk = sprintf(region,"%s:%d-%d",contig,1,end);
        check(chk==((strlen(contig)+get_int_length(end))+3),"Error creating region line from bed entry '%s'.",line);
        our_region_list[i] = region;
        i++;
      }//End of if this is an SQ line
      line = strtok(NULL,"\n");
    }
    bam_hdr_destroy(header);
    free(line);
    hts_close(bam);
	}

  check(no_of_regions>0,"Error evaluating regions to parse. None found.");

  //Now create a bigwigfile
  //Open file as a bw file
  chromList_t *chromList = NULL;
  //Generate the list of chromosomes using the bam header.
  chromList = build_chromList_from_bam_limit(input_file,contigs_h);
	check(chromList != NULL,"Error generating chromList from bam with limits");
	//fprintf(stderr,"Post build list entries: %d\t%s:%d\n",chromList->nKeys,chromList->chrom[0],chromList->len[0]);

	for (k = 0; k < kh_end(contigs_h); ++k)
		if (kh_exist(contigs_h, k))
			free((char*)kh_key(contigs_h, k));
	kh_destroy(str, contigs_h);

  check(chromList!=NULL,"Error building chromList from bam header.");

  //Initialise bw
  int res = bwInit(1<<17);
  check(res==0,"Received an error in bwInit");

  tmp.bwout = initialise_bw_output(out_file,chromList);
  check(tmp.bwout!= NULL,"Error initialising bw output file %s.",out_file);
  tmp.inczero=include_zeroes;
  uint32_t start;
  uint32_t stop;
  float cvg;
  //Now we generate the bw info
  int chck = 0;
	bw_func_reg func_reg = &pileup_func;
	if(is_overlap==1){
		func_reg = &pileup_func_overlap;
	}
  int i=0;
	for(i=0;i<no_of_regions;i++){
	  chck = process_bam_region(input_file, func_reg, &tmp, filter, filterinc, our_region_list[i], reference);
	  check(chck==1,"Error parsing bam region.");
	  start =  tmp.lstart;
    stop = tmp.lpos+1;
    cvg = tmp.lcoverage;
    cvg = _rescale(cvg,scale);
    if(start>0) {
      chck = bwAddIntervals(tmp.bwout,&tmp.head->target_name[tmp.ltid],&start,&stop,&cvg,single);
      check(chck==0,"Error adding bw interval %s:%"PRIu32"-%"PRIu32". TID: %d Error code: %d\n",tmp.head->target_name[tmp.ltid],start,stop,tmp.ltid,chck);
    }
	  if(include_zeroes == 1){
	    uint32_t reg_start;
	    uint32_t reg_stop;
	    char *contig = parseRegionString(our_region_list[i], &reg_start, &reg_stop);
	    uint32_t len = getContigLength(contig,chromList);
	    check(len != -1,"Error fetching length of contig %s.",contig);
	    //Append end of chromosome if zeroes
      if(strcmp(contig,last_contig) != 0 && tmp.lstart < reg_stop && tmp.lpos < reg_stop-1){
        start = stop;
        if(stop==1) start = 0;
        stop = reg_stop;
        float zero = 0;
        chck = bwAddIntervals(tmp.bwout,&contig,&start,&stop,&zero,single);
        check(chck==0,"Error adding bw interval %s:%"PRIu32"-%"PRIu32". Error code: %d\n",contig,start,stop,chck);
        tmp.lstart = stop;
        tmp.lcoverage = 0;
        tmp.lpos = len;
        tmp.ltid = 0;
      }
      free(contig);
	  }
	}

  bwClose(tmp.bwout);
  bwCleanup();

  int clean=0;
  for(clean=0; clean<chromList->nKeys; clean++){
    if(chromList->chrom){
      free(chromList->chrom[clean]);
    }
  }
  free(chromList->chrom);
  free(chromList->len);
  free(chromList);
  free(our_region_list);
  return 0;

error:
  if(our_region_list) free(our_region_list);
  if(fp_bed) fclose(fp_bed);
	if(contigs_h) {
		for (k = 0; k < kh_end(contigs_h); ++k){
			if (kh_exist(contigs_h, k)){
					free((char*)kh_key(contigs_h, k));
				}
			}
			kh_destroy(str, contigs_h);
	}
	if(chromList){
		for(clean=0; clean<sq_lines; clean++){
	    if(chromList->chrom){
	      free(chromList->chrom[clean]);
	    }
	  }
	  free(chromList->chrom);
	  free(chromList->len);
	  free(chromList);
	}
  return -1;
}
