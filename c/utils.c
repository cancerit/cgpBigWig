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
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <inttypes.h>
#include "htslib/sam.h"
#include "utils.h"


char *region_format = "%[^:]:%"SCNu32"-%"SCNu32"";

int check_exist(char *fname){
	FILE *fp;
	if((fp = fopen(fname,"r"))){
		fclose(fp);
		return 1;
	}
	return 0;
}

int parse_file_name( char *dir, char *fname, const char *file){
  char *tmp = alloca(sizeof(char) * strlen(file)+1);
  memset(dir,0,sizeof(dir));
  memset(fname,0,sizeof(fname));
  strcpy(tmp, file);
  int total_l = 0;
  if(strncmp(tmp,"./",2)==0){
    tmp = &tmp[2];
    total_l += 2;
  }
  char *ssc;
  int l = 0;

  ssc = strstr(tmp, "/");
  while(ssc){
    l = strlen(ssc) + 1;
    total_l += strlen(tmp)-l+2;
    tmp = &tmp[strlen(tmp)-l+2];
    ssc = strstr(tmp, "/");
  };

  fname = strcpy(fname,&file[total_l]);
  dir = strcat(dir,"");
  if(total_l){
    dir = strncat(dir,file,total_l);
  }
  strcat(fname,"\0");
  strcat(dir,"\0");
  return 1;
}

void print_version (int exit_code){
  printf ("%s\n",VERSION);
	exit(exit_code);
}

int line_count (char *file_path){
  FILE *f = fopen(file_path,"r");
  int line_count = 0;
  check(f != NULL, "Error opening file '%s' to count lines.",file_path);
  char rd[ 5000 ];
	while(fgets(rd, sizeof(rd), f) != NULL){
    line_count++;
  }
  fclose(f);
  return line_count;
error:
  if(f) fclose(f);
  return -1;
}

int parseRegionString(char *region, uint32_t *start, uint32_t *stop){
  const char *q = hts_parse_reg(region, start, stop);
  char *contig = (char*)malloc(q - region + 1);
  strncpy(contig, region, q - region);
  contig[q - region] = 0;
  *start = *start+1;
  return contig;
}