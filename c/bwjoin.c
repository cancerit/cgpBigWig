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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <regex.h>
#include "bigWig.h"
#include "utils.h"

char *out_file = "concatenated.bw";
char *fai = NULL;
char *path = NULL;
char *contig_input = NULL;
int ign_count = 0;

void print_usage (int exit_code){
	printf("Usage: bwjoin -f genome.fai -o output.bw\n\n");
	printf("-p  --input-path [dir]                           Path to the input bigwig files named [path]/<contig_name>.bw\n");
	printf("-f  --fasta-index [file]                         Fasta index file (.fai)\n");
	printf("-o  --outfile [file]                             Path to the output .bw file produced. [default:'%s']\n",out_file);
	printf("-g  --ignore-contigs [comma separated list]      Comma separated list of contigs to ignore.\n\n");
  printf ("Other:\n");
	printf ("-h --help      Display this usage information.\n");
	printf ("-v --version   Prints the version number.\n\n");

  exit(exit_code);
}

void setup_options(int argc, char *argv[]){
	const struct option long_opts[] =
	{
             	{"input-path", required_argument, 0, 'p'},
             	{"fasta-index", required_argument, 0, 'f'},
             	{"outfile",required_argument,0,'o'},
             	{"ignore-contigs",required_argument,0,'g'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}
   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "f:o:g:p:hv",long_opts, &index)) != -1){
    switch(iarg){
      case 'p':
        path = optarg;
        break;
   		case 'f':
				fai = optarg;
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
			case 'g':
			  contig_input = optarg;
			  break;
			case '?':
        print_usage (1);
        break;
      default:
      	print_usage (0);
   	}; // End of args switch statement

   }//End of iteration through options

  //Do some checking to ensure required arguments were passed and are accessible files
  if(check_exist(fai) != 1){
    fprintf(stderr,"Fasta index (.fai) file %s does not appear to exist.\n",fai);
    print_usage(1);
  }
}

char *build_regex_string(char *input){
  char *new = NULL;
  new = malloc(sizeof(char) * (strlen(input) + 50));
  int m=0;
  new[0] = '^';
  int pos = 1;
  for(m=0;m<strlen(input);m++){
    if(input[m]=='%'){
     new[pos]='.';
     pos++;
     new[pos]='*';
    }else{
      new[pos]=input[m];
    }
    pos++;
  }
  new[pos] = '$';
  pos++;
  new[pos] = '\0';
  return new;
}

char **parse_contig_list(char *contigs, char **chrNames, int noChrs){
  if(contigs == NULL) return NULL;
  char **ignore_list = NULL;
  char *copy = malloc(sizeof(char) * strlen(contigs)+1);
  strcpy(copy,contigs);
  char *tag = strtok(copy,",");
  ign_count=0;
  while(tag != NULL){
    if(strchr(tag,'%')){
      char *reg = build_regex_string(tag);
      regex_t regex;
      int reti = regcomp(&regex, reg, 0);
      if (reti) {
          fprintf(stderr, "Could not compile regex %s\n",reg);
          exit(1);
      }
      int k=0;
      for(k=0;k<noChrs;k++){
        reti = regexec(&regex, chrNames[k], 0, NULL, 0);
        if (!reti){
          ign_count++;
        }
      }
    }else{
      ign_count++;
    }
    tag = strtok(NULL,",");
  }
  ignore_list = malloc(sizeof(char *) * ign_count);
  char *tg = strtok(contigs,",");
  int idx=0;
  while(tg != NULL){
    if(strchr(tg,'%')){
      char *reg = build_regex_string(tg);
      regex_t regex;
      int reti = regcomp(&regex, reg, 0);
      if (reti) {
          fprintf(stderr, "Could not compile regex %s\n",reg);
          exit(1);
      }
      int k=0;
      for(k=0;k<noChrs;k++){
        reti = regexec(&regex, chrNames[k], 0, NULL, 0);
        if (!reti){
          ignore_list[idx] = malloc(sizeof(char) * strlen(chrNames[k])+1);
          strcpy(ignore_list[idx],chrNames[k]);
          idx++;
        }
      }
    }else{
      ignore_list[idx] = malloc(sizeof(char) * strlen(tg)+1);
      strcpy(ignore_list[idx],tg);
      idx++;
    }
    tg = strtok(NULL,",");
  }
  return ignore_list;
}

int check_ignore(char **ignore_list, int ign_size, char *contig){
  int i=0;
  for(i=0;i<ign_size;i++){
    if(strcmp(ignore_list[i],contig)==0) return 1;
  }
  return 0;
}

int main(int argc, char *argv[]){
  setup_options(argc, argv);

  char **ignore = NULL;
  //Open file as a bw file
  bigWigFile_t *bw_in = NULL;
  bwOverlappingIntervals_t *intervals = NULL;
  char **fileList = NULL;
  char **chrList = NULL;
  chromList_t *chromList = NULL;
  FILE *idx = NULL;
  bigWigFile_t *bw_out = NULL;

  int contig_count=line_count(fai);
  if(contig_count<0){
    fprintf(stderr,"Error getting line count from file %s\n",fai);
    goto error;
  }

  chromList = calloc(1, sizeof(chromList_t));
  if(!chromList){
    fprintf(stderr,"Error assigning memory for chromList\n");
    goto error;
  }

  chromList->nKeys = contig_count;
  chromList->chrom = malloc(contig_count * sizeof(char *));
  if(!chromList->chrom){
    fprintf(stderr,"Error assigning memory for chromList chrom\n");
    goto error;
  }
  chromList->len = malloc(contig_count * sizeof(uint32_t));
  if(!chromList->len){
    fprintf(stderr,"Error assigning memory for chromList len\n");
    goto error;
  }
  fileList=malloc(contig_count * sizeof(char *));
  if(!fileList){
    fprintf(stderr,"Error assigning memory for fileList\n");
    goto error;
  }

  idx = fopen(fai,"r");
  if(idx==NULL){
    fprintf(stderr,"Error opening input fai file %s for reading:%d.\n",fai,errno);
  }

  //Build list of chr names and filenames from fasta index
  char line [ 5000 ];
  int i=0;
	while ( fgets(line,sizeof(line),idx) != NULL ){
	  char contig [ 4000 ];
	  char filePath [5000] = "";
	  uint32_t len;
	  int chk = sscanf(line, "%s\t%d", contig,&len);
    if(chk<=0){
      fprintf(stderr,"Error fetching contig from fai at line %s\n",line);
    }
    chromList->chrom[i] = malloc(sizeof(char) * (strlen(contig)+1));
    if(!chromList->chrom[i]){
      fprintf(stderr,"Error allocating memory for chromList->chrom[%d]\n",i);
      goto error;
    }
    strcpy(chromList->chrom[i],contig);
    chromList->len[i] = len;
    //fileList[i] =
    strcpy(filePath,path);
    strcat(filePath,"/");
    strcat(filePath,contig);
    strcat(filePath,".bw");
    fileList[i] = malloc(sizeof(char) * (strlen(filePath)+1));
    if(!fileList[i]){
      fprintf(stderr,"Error allocating memory for fileList[%d]\n",i);
      goto error;
    }
    strcpy(fileList[i],filePath);
    i++;
	}
  //Close input file
  fclose(idx);

  ignore = parse_contig_list(contig_input,chromList->chrom,chromList->nKeys);

  //Open output file
  bw_out = bwOpen(out_file, NULL, "w");
  if(!bw_out) {
    fprintf(stderr, "An error occurred while opening %s for writing\n",out_file);
    return 1;
  }

  int j=0;
  int first=1;
  for(j=0;j<chromList->nKeys;j++){//Iterate through each contig and access file.

    if(ignore != NULL && check_ignore(ignore,ign_count,chromList->chrom[j])){
      continue;
    }
    //Open file as a bw file
    bw_in = NULL;
    //Initialize enough space to hold 128KiB (1<<17) of data at a time
    if(bwInit(1<<17) != 0) {
      fprintf(stderr, "Received an error in bwInit\n");
      return 1;
    }
    if(check_exist(fileList[j])!=1){
      fprintf(stderr,"BW file for reading '%s' doesn't seem to exist\n",fileList[j]);
    }

    bw_in = bwOpen(fileList[j], NULL, "r");
    if(!bw_in) {
      fprintf(stderr, "Error opening bw file %s\n",fileList[j]);
      goto error;
    }

    //Write the header to file
    if(first==1){
      if(bwCreateHdr(bw_out, bw_in->hdr->nLevels)){
        fprintf(stderr,"Error creating header for output bw file");
        goto error;
      }
      bw_out->cl = bwCreateChromList(chromList->chrom, chromList->len, chromList->nKeys);
      if(!bw_out->cl){
        fprintf(stderr,"Error creating chromlist for output fo file");
        goto error;
      }
      if(bwWriteHdr(bw_out)){
        fprintf(stderr,"Error writing header to file\n");
        goto error;
      }
      first=0;
    }

    intervals = NULL;
    intervals = bwGetOverlappingIntervals(bw_in, chromList->chrom[j], 0, chromList->len[j]);
    if(!intervals){
      fprintf(stderr,"Error retrieving intervals for %s:%d-%d\n",chromList->chrom[j], 0, chromList->len[j]);
      goto error;
    }

    if(intervals->l){
      chrList = NULL;
      chrList = malloc(intervals->l * sizeof(char *));
      if(!chrList){
        fprintf(stderr,"Error allocating memory for chromList\n");
        goto error;
      }
      uint64_t k=0;
      for(k=0;k<intervals->l;k++){
        chrList[k] = chromList->chrom[j];
      }
      if(bwAddIntervals(bw_out,chrList,intervals->start,intervals->end,intervals->value,intervals->l)){
        fprintf(stderr,"Error adding %d intervals for chromosome %s.\n",intervals->l,chromList->chrom[j]);
        goto error;
      }
      free(chrList);
    }
    bwDestroyOverlappingIntervals(intervals);
    bwClose(bw_in);

  }//End of iteration through each contig

  bwClose(bw_out);
  bwCleanup();
  int clean=0;
  for(clean=0; clean<chromList->nKeys; clean++){
    if(chromList->chrom){
      free(chromList->chrom[clean]);
    }
    if(fileList){
      free(fileList[clean]);
    }
  }
  free(chromList->len);
  free(chromList);
  return 0;

error:
  fclose(idx);
  bwClose(bw_out);
  if(intervals) bwDestroyOverlappingIntervals(intervals);
  if(bw_in){
    bwClose(bw_in);
    bwCleanup();
  }
  //TODO cleanup chromList object
  if(chromList){
    int clean=0;
    for(clean=0; clean<chromList->nKeys; clean++){
      if(chromList->chrom){
        free(chromList->chrom[clean]);
      }
      if(fileList){
        free(fileList[clean]);
      }
    }
    free(chromList->len);
    free(chromList);
  }
  return 1;

}