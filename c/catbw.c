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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bigWig.h"
#include "utils.h"
#include "dbg.h"


char *input = NULL;
char *output = NULL;
char *region = NULL;
int inc_na = 0;
char *out_pattern = "%s\t%"PRIu32"\t%"PRIu32"\t%f\n";

void print_usage (int exit_code){
	printf ("Usage: bwcat -i input-path\n\n");
	printf ("-i  --input-path [file]                          Path to the input bigwig file\n\n");
	printf ("Optional:\n");
	printf ("-r  --region [string]                            Region or bw file to print to screen format. NB start should be 0 based: (contig:start-stop)\n");
	printf ("-n  --include-na                                 Include NA regions in output\n");
	printf ("-o  --output [file]                              File to write bw output to [default: stdout]\n\n");
  printf ("Other:\n");
	printf ("-h --help      Display this usage information.\n");
	printf ("-v --version   Prints the version number.\n\n");

  exit(exit_code);
}

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"input-path", required_argument, 0, 'i'},
             	{"region",required_argument,0,'r'},
             	{"output",required_argument,0,'o'},
             	{"include-na",no_argument,0,'n'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}
   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "i:r:o:nhv",long_opts, &index)) != -1){
    switch(iarg){
      case 'i':
        input = optarg;
        break;
   		case 'o':
				output = optarg;
   			break;
			case 'h':
				print_usage (0);
				break;
			case 'v':
				print_version (0);
				break;
			case 'r':
			  region = optarg;
			  break;
			case 'n':
        inc_na = 1;
			  break;
			case '?':
        print_usage (1);
        break;
      default:
      	print_usage (0);
   	}; // End of args switch statement

   }//End of iteration through options

  //Do some checking to ensure required arguments were passed and are accessible files
  if(check_exist(input) != 1){
    fprintf(stderr,"Input bw file %s does not exist.\n",input);
    print_usage(1);
  }
  if(output==NULL || strcmp(output,"/dev/stdout")==0) {
    output = "-";   // we recognise this as a special case
  }
  return;
}

int main(int argc, char *argv[]){
  setup_options(argc, argv);
  bigWigFile_t *fp = NULL;
  bwOverlappingIntervals_t *intervals = NULL;
  FILE * out = NULL;

  if (strcmp(output,"-")==0) {
    out = stdout;
  } else {
    out = fopen(output,"w");
  }
  check(out != NULL, "Failed to open output file for %s writing.",output);

  //Initialize enough space to hold 128KiB (1<<17) of data at a time
  int chk = bwInit(1<<17);
  check(chk==0,"Received an error in bwInit");
  //Open the local/remote file
  fp = bwOpen(input, NULL, "r");
  check(fp!=NULL,"Error opening input bw file %s.",input);

  if(region!=NULL){
    //Check and parse region
    uint32_t start = 0;
    uint32_t stop = 0;
    char *contig = NULL;
    contig = parseRegionString(region,&start,&stop);
    check(contig!=NULL,"Error parsing region string '%s'",region);
    //retrieve region intervals
    //intervals = bwGetValues(fp, contig, start, stop, inc_na);
    intervals = bwGetOverlappingIntervals(fp, contig, start-1, stop);
    //Iterate through intervals
    if(intervals){
      uint32_t j=0;
      for(j=0;j<intervals->l;j++){
        //print interval
        fprintf(out,out_pattern,contig,(intervals->start)[j],(intervals->end)[j],(intervals->value)[j]);
      }
      bwDestroyOverlappingIntervals(intervals);
    }
    free(contig);
  }else{
    //No region so iterate through each contig listed in the header file
    //Read in the list of chromosomes
    int i=0;
    //Iterate through each contig
    for(i=0;i<fp->cl->nKeys;i++){
      //intervals = bwGetValues(fp, fp->cl->chrom[i], 0, fp->cl->len[i], inc_na);
      intervals = bwGetOverlappingIntervals(fp, fp->cl->chrom[i], 0, fp->cl->len[i]);
      if(intervals){
        int j=0;
        for(j=0;j<intervals->l;j++){
          fprintf(out,out_pattern,fp->cl->chrom[i],(intervals->start)[j],(intervals->end)[j],(intervals->value)[j]);
        }
      }
    }
  }

  fflush(out);
  fclose(out);
  bwClose(fp);
  bwCleanup();
  return 0;

error:
  if(intervals) bwDestroyOverlappingIntervals(intervals);
  if(out) fclose(out);
  if(fp) bwClose(fp);
  return -1;
}
