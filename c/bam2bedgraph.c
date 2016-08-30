#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "htslib/sam.h"

static char *input_file = NULL;
static char *output_file = NULL;
static char *region = NULL;
static int filter = 0;

typedef struct {
  uint32_t ltid;
  int      lstart,lcoverage,lpos,beg,end;
  htsFile *in;
  hts_idx_t *idx;
	bam_hdr_t *head;
	FILE *out;
} tmpstruct_t;

void print_version (int exit_code){
  printf ("%s\n",VERSION);
	exit(exit_code);
}

int check_exist(char *fname){
	FILE *fp;
	if((fp = fopen(fname,"r"))){
		fclose(fp);
		return 1;
	}
	return 0;
}

void print_usage (int exit_code){

	printf ("Usage: bam2bedgraph -i input.[cr|b]am -o file [-r region] [-h] [-v]\n\n");
	printf ("Create a BEDGraph file of genomic coverage. BAM file must be sorted.\n");
  printf ("-i --input     Path to bam/cram input file. [default: stdin]\n");
  printf ("-o --output    File path for output. [default: stdout]\n\n");
	printf ("Optional:\n");
	printf ("-r --region    Region in bam to access.\n");
	printf ("-f --filter    Ignore reads with the filter flags [int].\n");
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
              {"rna",no_argument,0, 'a'},
              { NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "i:o:r:f:vh", long_opts, &index)) != -1){
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
static int pileup_func(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data)
{
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

int main(int argc, char *argv[]){
	options(argc, argv);
  tmpstruct_t tmp;
  bam_plp_t buf = NULL;
	bam1_t *b = NULL;
  hts_itr_t *iter = NULL;
  tmp.beg = 0; tmp.end = 0x7fffffff;
	tmp.lstart    = 0;
	tmp.lcoverage = 0;
	tmp.ltid      = 0;
	tmp.lpos      = 0;

	if (strcmp(output_file,"-")==0) {
    tmp.out = stdout;
  } else {
    tmp.out = fopen(output_file,"w");
  }
	if(tmp.out == NULL){
	  fprintf(stderr,"Failed to open output file for %s writing.",output_file);
	  return 1;
	}
  tmp.in = hts_open(input_file, "r");
  if (tmp.in == 0) {
		fprintf(stderr, "Fail to open [CR|B]AM file %s\n", input_file);
		return 1;
	}
  tmp.head = sam_hdr_read(tmp.in);
  buf = bam_plp_init(0,0);
  //Iterate through each read in bam file.
  b = bam_init1();
	if(region == NULL){
	  tmp.idx=NULL;
    int reto;
    while((reto = sam_read1(tmp.in, tmp.head, b)) >= 0){
      if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
      int ret, n_plp, tid, pos;
      const bam_pileup1_t *plp;
      ret = bam_plp_push(buf, b);
      if (ret < 0) break;
      while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0)
          pileup_func(tid, pos, n_plp, plp, &tmp);
    }
    bam_plp_push(buf,0);
	}else{

	  tmp.idx = sam_index_load(tmp.in,input_file);
    if (tmp.idx == 0) {
		  fprintf(stderr, "Fail to open [CR|B]AM index for file %s\n", input_file);
		  return 1;
	  }
    iter = sam_itr_querys(tmp.idx, tmp.head, region);
    int result;
    while ((result = sam_itr_next(tmp.in, iter, b)) >= 0) {
      if((b->core.flag & filter)>0) continue; //Skip if this is a filtered read
      int ret, n_plp, tid, pos;
      const bam_pileup1_t *plp;
      ret = bam_plp_push(buf, b);
      if (ret < 0) break;
      while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
          pileup_func(tid, pos, n_plp, plp, &tmp);
      }
    }
    bam_plp_push(buf,0);
	}

  //Check we've written everything...
	int n_plp, tid, pos;
  const bam_pileup1_t *plp;
  while ((plp = bam_plp_next(buf, &tid, &pos, &n_plp)) != 0){
    pileup_func(tid, pos, n_plp, plp, &tmp);
  }

  fprintf(tmp.out,"%s\t%d\t%d\t%d\n", tmp.head->target_name[tmp.ltid], tmp.lstart,tmp.lpos+1, tmp.lcoverage);
  bam_plp_destroy(buf);
  bam_destroy1(b);
  fflush(tmp.out);
  fclose(tmp.out);
  if(iter) sam_itr_destroy(iter);
  if(tmp.idx) hts_idx_destroy(tmp.idx);
	if(tmp.in) hts_close(tmp.in);
	if(tmp.head) bam_hdr_destroy(tmp.head);
	return 0;
}

