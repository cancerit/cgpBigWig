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
#include "bigWig.h"
#include "utils.h"
#include "dbg.h"

char *input_file = NULL;
char *out_file = "output.bw";
char *chrlist = NULL;

void print_usage (int exit_code){
	printf("Usage: bg2bw -i input.bed -c chrom.list -o output.bw\n");
	printf("bg2bw can be used to generate a bw file from a bedgraph file.\n\n");
	printf("-i  --input [file]           Path to the input [b|cr]am file.\n");
	printf("-o  --outfile [file]         Path to the output .bw file produced. [default:'%s']\n",out_file);
	printf("-c  --chromList [file]       Path to chrom.list a .tsv where first two columns are contig name and length.\n\n");
  printf ("Other:\n");
	printf("-h  --help                   Display this usage information.\n");
	printf("-v  --version                Prints the version number.\n\n");

  exit(exit_code);
}

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"input", required_argument, 0, 'i'},
             	{"outfile",required_argument, 0, 'o'},
             	{"chromList",required_argument, 0, 'c'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "c:i:o:hv",long_opts, &index)) != -1){
    switch(iarg){
   		case 'i':
				input_file = optarg;
				if(check_exist(input_file) != 1){
          fprintf(stderr,"Input bed file %s does not appear to exist.\n",input_file);
          print_usage(1);
        }
   			break;

   		case 'c':
   		  chrlist = optarg;
   		  if(check_exist(chrlist) != 1){
          fprintf(stderr,"Input chrlist file %s does not appear to exist.\n",chrlist);
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
			case '?':
        print_usage (1);
        break;
      default:
      	print_usage (0);
   	}; // End of args switch statement

   }//End of iteration through options

  if(input_file==NULL){
    fprintf(stderr,"Required option -i|--input not defined.\n");
    print_usage(1);
  }

  if(chrlist==NULL){
    fprintf(stderr,"Required option -c|--chrList not defined.\n");
    print_usage(1);
  }

  return;
}

chromList_t *parse_chrom_list(char *chrom_list_file){
  chromList_t *cl = NULL;
  FILE *in = NULL;
  char **contigs = NULL;
  uint32_t *lengths = NULL;
  int ln_count = line_count(chrom_list_file);
  contigs = malloc(sizeof(char *) * ln_count);
  check_mem(contigs);
  lengths = malloc(sizeof(uint32_t *) * ln_count);
  check_mem(lengths);
  int chr_count = 0;
  in = fopen(chrom_list_file,"r");
  check(in!=NULL,"Error opening chrom list file '%s' for reading.",chrom_list_file);
  char line[2048];
  //Iterate through each line in the chromlist file
  while(fgets(line, sizeof(line),in)){
    char *cntg = malloc(sizeof(char *) * 2048);
    check_mem(cntg);
    uint32_t len;
    int no = sscanf(line,"%[^\t]\t%"SCNu32"\t",cntg,&len);
    check(no==2,"Error extracting contig name and length from line %s.",line);
    contigs[chr_count] = cntg;
    lengths[chr_count] = len;
    chr_count++;
  }
  check(chr_count==ln_count,"Error contigs found %d != line_count %d",chr_count,ln_count);
  fclose(in);
  cl = bwCreateChromList(contigs, lengths, chr_count);
  check(cl!=NULL,"Error creating ChromList.");
  return cl;

error:
  if(in) fclose(in);
  if(contigs) free(contigs);
  if(lengths) free(lengths);
  return NULL;
}

int main(int argc, char *argv[]){
  setup_options(argc, argv);
  FILE *in = NULL;
  bigWigFile_t *fp = NULL;
  char *ctg = NULL;
  in = fopen(input_file,"r");
  check(in != NULL, "Error opeining input bed file '%s' to read.",input_file);

  //Read in the chromlist
  int chk = bwInit(1<<17);
  check(chk==0,"Received an error in bwInit: %d",chk);

  fp = bwOpen(out_file, NULL, "w");
  check(fp!=NULL,"Error opening output bw file '%s' for writing.",out_file);

  chk = bwCreateHdr(fp, 10);
  check(chk==0,"Error creating bw header: %d.",chk);

  fp->cl = parse_chrom_list(chrlist);
  check(fp->cl!=NULL,"Error parsing chrom list for contig information.");

  chk = bwWriteHdr(fp);
  check(chk==0,"Error writing bw header: %d.",chk);

  char line[2048];
  int num = 0;
  ctg = malloc(sizeof(char) * 2048);
  uint32_t start;
  uint32_t stop;
  float res;
  while(fgets(line,sizeof(line),in)){
    num = sscanf(line,"%[^\t]\t%"SCNu32"\t%"SCNu32"\t%f\n",ctg,&start,&stop,&res);
    check(num==4,"Error parsing bed line '%s' to bw format.",line);
    chk = bwAddIntervals(fp, &ctg, &start, &stop, &res, 1);
    check(chk==0,"Error encountered adding bed line '%s' to bw file: %d.",line,chk);
  }

  free(ctg);
  fclose(in);
  bwClose(fp);
  bwCleanup();
  return 0;
error:
  if(fp) bwClose(fp);
  bwCleanup();
  if(ctg) free(ctg);
  if(in) fclose(in);
  return 1;
}

