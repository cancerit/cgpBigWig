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

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <libgen.h>
#include "bam_access.h"
#include "utils.h"

char *out_file = NULL;
char *input_file = NULL;
char *region_store = NULL;
char **our_region_list = NULL;
char *reference = NULL;
uint32_t region_list_count = 0;
int is_regions_file = 0;
uint8_t is_base = 0;
int filter = 4;
char base = 0;
uint32_t single = 1;

void print_usage (int exit_code){
	printf("Usage: bam2bwbases -i input.[b|cr]am -o output.bw\n");
	printf("bam2bwbases can be used to generate four bw files of per base proportions.\n\n");
	printf("-i  --input [file]                                Path to the input [b|cr]am file.\n");
	printf("-F  --filter [int]                                SAM flags to filter. [default: %d]\n",filter);
	printf("-o  --outfile [file]                              Path to the output .bw file produced. Per base results wiillbe output as four bw files [ACGT].outputname.bw [default:'%s']\n\n",out_file);
	printf("Optional: \n");
	printf("-c  --region [file]                               A samtools style region (contig:start-stop) or a bed file of regions over which to produce the bigwig file\n");
	printf("-r  --reference [file]                            Path to reference genome.fa file (required for cram if ref_path cannot be resolved)\n\n");
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

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"input", required_argument, 0, 'i'},
             	{"filter", required_argument, 0, 'F'},
             	{"outfile",required_argument, 0, 'o'},
             	{"region",required_argument, 0, 'c'},
             	{"reference",required_argument, 0, 'r'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "F:i:o:c:r:hv",long_opts, &index)) != -1){
    switch(iarg){
      case 'F':
        if(sscanf(optarg, "%i", &filter) != 1){
      		fprintf(stderr,"Error parsing -F|--filter argument '%s'. Should be an integer > 0",optarg);
      		print_usage(1);
      	}
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

  if(out_file == NULL){
    out_file = "output.bam.bw";
  }
  return;
}

// callback for bam_plbuf_init()
static int perbase_pileup_func(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data, uint32_t reg_start){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos              = (int)position;
  int coverage         = n;
  int base_coverage    = 0;
  int i;
  for (i=0;i<n;i++){
    if (pl[i].is_del){
      coverage--;
    }else{
      if(bam_seqi(bam_get_seq(pl[i].b), pl[i].qpos) == tmp->base_bit) base_coverage++;
    }
  }
  float result = 0;
  if(base_coverage>0) result = (float)base_coverage / (float) coverage;
  if((uint32_t)pos == reg_start-1){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
    tmp->lbaseprop  = result;
  }
  if(tmp->ltid != tid || tmp->lbaseprop != result || pos > tmp->lpos+1){
    //if(tmp->lpos > 0){
      uint32_t start =  tmp->lstart;
      uint32_t stop = pos;
      float res = tmp->lbaseprop;
      if(start >= tmp->reg_start-1 && stop <= tmp->reg_stop){
        int chck = bwAddIntervals(tmp->bwout,&tmp->head->target_name[tmp->ltid],&start,&stop,&res,single);
        if(chck!=0){
          fprintf(stderr,"Error adding region to bw '%s:%"PRIu32"-%"PRIu32"\t%f'. Error code: %d.\n",tmp->head->target_name[tmp->ltid],start,stop,res,chck);
          exit(1);
        }
      }
    //}
    tmp->ltid          = tid;
    tmp->lstart        = pos;
    tmp->lbaseprop     = result;
  }
  tmp->lpos = pos;
  return 0;
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

int main(int argc, char *argv[]){
	setup_options(argc, argv);
	tmpstruct_t *perbase = NULL;
	int no_of_regions = 0;
	int sq_lines = get_no_of_SQ_lines(input_file);
	FILE *fp_bed = NULL;
	char *dir = NULL;
  char *fname = NULL;
	chromList_t *chromList = NULL;
	if(region_store != NULL){
    if(is_regions_file==1){
      //If we have a bedfile
      no_of_regions = line_count(region_store);
      check(no_of_regions>0,"Error counting entries in region file %s\n",region_store);
      our_region_list = calloc(no_of_regions, sizeof(char *));
      check_mem(region_list);
      int i=0;
      fp_bed = fopen(region_store,"r");
        char line[512];
        while(fgets(line,512,fp_bed)){
          char *contig = malloc(sizeof(char)*256);
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
    }else{
      //If we have a single region....
      no_of_regions = 1;
      our_region_list = calloc(no_of_regions, sizeof(char *));
      check_mem(region_list);
      our_region_list[0] = region_store;
    }
	}else{
    //If not...
    //Build a list of regions from the header of the bamfile
    no_of_regions = sq_lines;
    our_region_list = calloc(no_of_regions, sizeof(char *));
    htsFile *bam =  NULL;
    char *line = NULL;
    bam = hts_open(input_file, "r");
    check(bam != 0,"Bam file %s failed to open to read header.",input_file);
    bam_hdr_t *header = sam_hdr_read(bam);
    char *head_txt = header->text;
    line = strtok(head_txt,"\n");
    int i=0;
    while(line != NULL){
      //Check for a read group line
      if(strncmp(line,"@SQ",3)==0){
        char *contig = malloc(sizeof(char) * 256);
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
  //Generate the list of chromosomes using the bam header.
  chromList = calloc(1, sizeof(chromList_t));
  check_mem(chromList);
  chromList->nKeys = sq_lines;
  chromList->chrom = calloc(sq_lines,sizeof(char *));
  check_mem(chromList->chrom);
  chromList->len = calloc(sq_lines,sizeof(uint32_t));
  check_mem(chromList->len);
  //Iterate through the header of the bam file and get all contigs.
  int res = build_chromList_from_bam(chromList,input_file);
  check(res==1,"Error building chromList from bam header.");

  //Initialise bw
  res = bwInit(1<<17);
  check(res==0,"Received an error in bwInit");


  uint8_t base_bit_list[] = {1,2,4,8};
  char *base_list[] = {"A","C","G","T"};
  perbase = malloc(4 * sizeof(tmpstruct_t));
  check_mem(perbase);
  fname = malloc(sizeof(char) * strlen(out_file) +1 );
  dir = malloc(sizeof(char) * strlen(out_file) +1 );
  parse_file_name(dir,fname,out_file);
  int l=0;
  for(l=0;l<4;l++){
    perbase[l].lbaseprop = 0;
    perbase[l].base_bit = base_bit_list[l];
    char *name = malloc(sizeof(char *) * (strlen(out_file)+3));
    check_mem(name);
    strcpy(name,dir);
    strcat(name,base_list[l]);
    strcat(name,".");
    strcat(name,fname);
    perbase[l].bwout = initialise_bw_output(name,chromList);
    check(perbase[l].bwout != NULL,"Error initialising bw output file %s.",name);
    free(name);
    perbase[l].beg = 0; perbase[l].end = 0x7fffffff;
    perbase[l].lstart    = 0;
    perbase[l].lcoverage = 0;
    perbase[l].ltid      = 0;
    perbase[l].lpos      = 0;
  }

  //Now we generate the bw info
  int chck = 0;
  uint32_t sta;
	uint32_t sto;
	char *contig = malloc(sizeof(char) * 2048);
  int i=0;
	for(i=0;i<no_of_regions;i++){
	  parseRegionString(our_region_list[i], contig, &sta, &sto);
	  int k=0;
    for(k=0;k<4;k++){
      perbase[k].reg_start = sta;
      perbase[k].reg_stop = sto;
    }
	  chck = process_bam_region_bases(input_file, perbase_pileup_func, perbase, filter,  our_region_list[i], reference);
	  check(chck==1,"Error processing bam region.");
    int b=0;
    for(b=0;b<4;b++){
      float result = perbase[b].lbaseprop;
      uint32_t start = perbase[b].lstart;
      uint32_t stop = perbase[b].lpos+1;
      if(start <= sto-1){
        if(stop > sto) stop = sto;
        chck = bwAddIntervals(perbase[b].bwout,&perbase[b].head->target_name[perbase[b].ltid],&start,&stop,&result,single);
        check(chck==0,"Error adding region to bw '%s:%"PRIu32"-%"PRIu32"\t%f'. TID: %d\tErrno: %d",perbase[b].head->target_name[perbase[b].ltid],start,stop,result,perbase[b].ltid,chck);
      }
      if(stop < sto){
        result = 0.0;
        start = stop;
        stop = sto;
        chck = bwAddIntervals(perbase[b].bwout,&perbase[b].head->target_name[perbase[b].ltid],&start,&stop,&result,single);
        check(chck==0,"Error adding region to bw '%s:%"PRIu32"-%"PRIu32"\t%f'. TID: %d\tErrno: %d",perbase[b].head->target_name[perbase[b].ltid],start,stop,result,perbase[b].ltid,chck);
      }
    }
    bam_hdr_destroy(perbase[0].head);
	}


  int b=0;
  for(b=0;b<4;b++){
    bwClose(perbase[b].bwout);
  }
  bwCleanup();
  int clean=0;
  for(clean=0; clean<sq_lines; clean++){
    if(chromList->chrom){
      free(chromList->chrom[clean]);
    }
  }
  free(chromList->chrom);
  free(chromList->len);
  free(chromList);
  free(perbase);
  free(our_region_list);
  return 0;

error:
  if(our_region_list) free(our_region_list);
  if(fp_bed) fclose(fp_bed);
  if(chromList){
    int clean=0;
    for(clean=0; clean<sq_lines; clean++){
      if(chromList->chrom){
        free(chromList->chrom[clean]);
      }
    }
    free(chromList->chrom);
    free(chromList->len);
    free(chromList);
  }
  if(perbase) free(perbase);
  return -1;
}
