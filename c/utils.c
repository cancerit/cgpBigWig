#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <inttypes.h>
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

int parseRegionString(char *region, char *contig, uint32_t *start, uint32_t *stop){
  int check_parse = sscanf(region,region_format,contig,start,stop);
  if(check_parse != 3) return 0;
  return check_parse;
}