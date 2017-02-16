/**   LICENSE
* Copyright (c) 2017 Genome Research Ltd.
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
#include <libgen.h>
#include <string.h>
#include "bigWig.h"
#include "utils.h"

char *outdir = NULL;
char *bigwig = NULL;
char *ref = NULL;
char *decode = NULL;
int sd = 12;

void print_usage (int exit_code){
	printf("Usage: detectExtremeDepth  - Generate profile of BigWig file and identify regions outside the normal range\n\n");
	printf("-b  --bigwig [file]     BigWig file path.\n");
	printf("-o  --output [dir]      Folder to send output to\n");
	printf("                          - named as input file with '.tab' extension\n\n");
	printf("Optional:\n");
	printf("-r  --ref [str]         Restrict to this reference (mainly for testing)\n");
  printf("                          - without 'chr' prefix, will test with and without the 'chr' for you.\n");
	printf("                          - if '-r' defined '.{val}' will prefix '.bed'\n");
	printf("-s  --sd [int]          Number of standard deviations above mean for group to be included [%d].\n",sd);
	printf("-d  --decode [str]      Decode -r to chromosome names (do not include 'chr')\n");
	printf("                          e.g. -d 23:X -d 24:Y -d 25:MT\n\n");
  printf("Other:\n");
	printf("-h  --help              Display this usage information.\n");
	printf("-v  --version           Prints the version number.\n\n");

  exit(exit_code);
}

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"bigwig", required_argument, 0, 'b'},
             	{"ref", required_argument, 0, 'r'},
             	{"output",required_argument,0,'o'},
             	{"decode",required_argument,0,'d'},
             	{"sd",required_argument,0,'s'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}
   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "r:s:o:b:d:hv",long_opts, &index)) != -1){
    switch(iarg){
      case 'b':
        bigwig = optarg;
        break;
   		case 'r':
				ref = optarg;
   			break;
   		case 'o':
				outdir = optarg;
   			break;
			case 'h':
				print_usage (0);
				break;
			case 'v':
				print_version (0);
				break;
			case 'd':
			  decode = optarg;
			  break;
			case 's':
			  if(sscanf(optarg, "%i", &sd) != 1){
      		fprintf(stderr,"Error parsing -s|--sd argument '%s'. Should be an integer > 0",optarg);
      		print_usage(1);
      	}
			  break;
			case '?':
        print_usage (1);
        break;
      default:
      	print_usage (0);
   	}; // End of args switch statement

   }//End of iteration through options

  //Do some checking to ensure required arguments were passed and are accessible files
  if(outdir == NULL){
    fprintf(stderr,"Output directory -o|output should be defined.\n");
    print_usage(1);
  }

  if(check_exist(bigwig)!= 1){
    fprintf(stderr,"Bigwig file %s does not appear to exist.\n",ref);
    print_usage(1);
  }
}

char *find_last_dot(char* str){
  char *ptr = str;
  char *dot = NULL;
  while(*ptr++){
    if (*ptr == '.'){
      dot = ptr;
    }
  }
  return dot;
}

int main(int argc, char *argv[]){
  setup_options(argc, argv);
  bigWigFile_t *fp = NULL;
  bwOverlappingIntervals_t *intervals = NULL;
  char *newfname = NULL;
  FILE *out = NULL;
  //Initialize enough space to hold 128KiB (1<<17) of data at a time
  if(bwInit(1<<17) != 0) {
    fprintf(stderr, "Received an error in bwInit\n");
    return 1;
  }

  //Open the local/remote file
  fp = bwOpen(bigwig, NULL, "r");
  if(!fp) {
    fprintf(stderr, "Error opening %s\n", bigwig);
    return 1;
  }

  //Open the output file.
  char *filename = basename(bigwig);
  newfname  = malloc(sizeof(char) * (strlen(filename)+2));
  if(!newfname){
    fprintf(stderr,"Error assigning memory for new filename.");
    goto error;
  }
  strcpy(newfname, filename);
  char *lastdot = find_last_dot(newfname);
  strcpy(lastdot,".bed");

  //Add output directory to new filename
  char *outputloc = malloc(sizeof(char) * (strlen(outdir) + strlen(newfname) + 2));
  strcpy(outputloc,outdir);
  strcat(outputloc,"/");
  strcat(outputloc,newfname);

  out = fopen(outputloc, "w");
  if(!out){
    fprintf(stderr,"Error opening output file %s.\n",outputloc);
    goto error;
  }
  //Iterate through each chromosome in this file
  int64_t i=0;
  for(i=0;i<fp->cl->nKeys;i++){
    double *mn = bwStats(fp,fp->cl->chrom[i],0, fp->cl->len[i],1,mean);
    double *stdv = bwStats(fp,fp->cl->chrom[i],0, fp->cl->len[i],1,stdev);

    fprintf(stderr,"%s: mean %.2f, stdev %.2f\n",fp->cl->chrom[i],mn[0],stdv[0]);
    intervals = bwGetOverlappingIntervals(fp, fp->cl->chrom[i], 0, fp->cl->len[i]);
    if(!intervals){
      fprintf(stderr,"Error retrieving intervals for %s:%d-%d\n",fp->cl->chrom[i], 0, fp->cl->len[i]);
      goto error;
    }
    if(intervals->l){
      double max_val = mn[0] + (stdv[0] * sd);
      fprintf(stderr,"%s: Max depth permitted = %.2f\n", fp->cl->chrom[i], max_val);
      uint64_t k=0;
      for(k=0;k<intervals->l;k++){
        if(intervals->value[k] > max_val){//If this exceeds our limit
          //Write to output file
          int writ = fprintf(out,"%s\t%d\t%d\t%.0f\n",fp->cl->chrom[i],intervals->start[k],intervals->end[k],intervals->value[k]);
          if(!writ){
            fprintf(stderr,"Error writing extreme depth interval to output file %s:%d-%d\n",fp->cl->chrom[i], 0, fp->cl->len[i]);
            goto error;
          }
        }
      }
    }//End of iteration through intervals in this chr
    bwDestroyOverlappingIntervals(intervals);
  }

  bwClose(fp);
  bwCleanup();
  fflush(out);
  fclose(out);
  free(newfname);
  return 0;

error:
  if(out) fclose(out);
  if(newfname) free(newfname);
  if(intervals) bwDestroyOverlappingIntervals(intervals);
  if(fp) bwClose(fp);
  bwCleanup();
  return 1;

}
