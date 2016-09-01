#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "bigWig.h"
#include "dbg.h"


char *input = NULL;
char *output = NULL;
char *region = NULL;
int inc_na = 0;
char *out_pattern = "%s\t%"PRIu32"\t%"PRIu32"\t%f\n";
char *region_format = "%[^:]:%"SCNu32"-%"SCNu32"";

int check_exist(char *fname){
	FILE *fp;
	if((fp = fopen(fname,"r"))){
		fclose(fp);
		return 1;
	}
	return 0;
}

void print_version (int exit_code){
  printf ("%s\n",VERSION);
	exit(exit_code);
}

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
    fprintf(stderr,"Input bw filw file %s does not exist.\n",input);
    print_usage(1);
  }
  if(output==NULL || strcmp(output,"/dev/stdout")==0) {
    output = "-";   // we recognise this as a special case
  }
  return;
}

int parseRegionString(char *region, char *contig, uint32_t *start, uint32_t *stop){
  int check_parse = sscanf(region,region_format,contig,start,stop);
  if(check_parse != 3) return 0;
  return check_parse;
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
    char contig[1024]= "";
    uint32_t start,stop;
    int chk = parseRegionString(region,contig,&start,&stop);
    check(chk>0,"Error parsing region string '%s'",region);
    //retrieve region intervals
    //intervals = bwGetValues(fp, contig, start, stop, inc_na);
    intervals = bwGetOverlappingIntervals(fp, contig, start, stop);
    //Iterate through intervals
    if(intervals){
      uint32_t j=0;
      for(j=0;j<intervals->l;j++){
        //print interval
        fprintf(out,out_pattern,contig,(intervals->start)[j],(intervals->end)[j],(intervals->value)[j]);
      }
      bwDestroyOverlappingIntervals(intervals);
    }
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