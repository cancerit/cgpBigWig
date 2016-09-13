#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <alloca.h>
#include <inttypes.h>
#include "bam_access.h"
#include "utils.h"

char *out_file = "output.bam.bw";
char *input_file = NULL;
char *region_store = NULL;
char **our_region_list = NULL;
char *reference = NULL;
uint32_t region_list_count = 0;
int is_regions_file = 0;
uint8_t is_base = 0;
int filter = 4;
char base = 0;
int include_zeroes = 0;
uint32_t single = 1;
char *last_contig = "";

void print_usage (int exit_code){
	printf("Usage: bam2bw -i input.[b|cr]am -o output.bw\n");
	printf("bam2bw can be used to generate a bw file of coverage from a [cr|b]am file.\n\n");
	printf("-i  --input [file]                                Path to the input [b|cr]am file.\n");
	printf("-F  --filter [int]                                SAM flags to filter. [default: %d]\n",filter);
	printf("-o  --outfile [file]                              Path to the output .bw file produced. [default:'%s']\n\n",out_file);
	printf("Optional: \n");
	printf("-c  --region [file]                               A samtools style region (contig:start-stop) or a bed file of regions over which to produce the bigwig file\n");
	printf("-z  --include-zeroes                              Include zero coverage regions as additional entries to the bw file\n");
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
             	{"include-zeroes",no_argument, 0, 'z'},
             	{"help", no_argument, 0, 'h'},
             	{"version", no_argument, 0, 'v'},
             	{ NULL, 0, NULL, 0}

   }; //End of declaring opts

   int index = 0;
   int iarg = 0;

   //Iterate through options
   while((iarg = getopt_long(argc, argv, "F:i:o:c:r:zhv",long_opts, &index)) != -1){
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
			case 'z':
			  include_zeroes  = 1;
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
static int pileup_func(uint32_t tid, uint32_t position, int n, const bam_pileup1_t *pl, void *data){
  tmpstruct_t *tmp = (tmpstruct_t*)data;
  int pos          = (int)position;
  int coverage     = n;
  int i;
  for (i=0;i<n;i++)
    if (pl[i].is_del) coverage--;
  if (tmp->ltid != tid || tmp->lcoverage != coverage || pos > tmp->lpos+1) {
    if ((tmp->inczero == 0 && tmp->lcoverage > 0 ) || tmp->inczero == 1){

      uint32_t start =  tmp->lstart;
      uint32_t stop;
      if(tmp->lpos > 0){
        stop = tmp->lpos +1;
      }else{
        tmp->ltid = tid;
        stop = pos;
      }
      float cvg = (float)tmp->lcoverage;
      int chck = bwAddIntervals(tmp->bwout,&tmp->head->target_name[tmp->ltid],&start,&stop,&cvg,single);
      if(chck != 0){
        fprintf(stderr,"Error adding region to bw '%s:%"PRIu32"-%"PRIu32"\t%f'. TID: %d\tErrno: %d\n",tmp->head->target_name[tmp->ltid],start,stop,cvg,tmp->ltid,chck);
        exit(1);
      }
    }
    //if(tmp->inczero == 1 && tmp->ltid != tid && pos != tmp->head->target_len[tmp->ltid]){
    tmp->ltid       = tid;
    tmp->lstart     = pos;
    tmp->lcoverage  = coverage;
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

char *get_contig_from_region(char *region){
  int beg = 0;
  int end = 0;
  const char *q = hts_parse_reg(region, &beg, &end);
  char *tmp = (char*)malloc(q - region + 1);
  strncpy(tmp, region, q - region);
  tmp[q - region] = 0;
  return tmp;
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
	int sq_lines = get_no_of_SQ_lines(input_file);
	FILE *fp_bed = NULL;
	if(region_store){
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
  chromList_t *chromList = NULL;
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

  tmp.bwout = initialise_bw_output(out_file,chromList);
  check(tmp.bwout!= NULL,"Error initialising bw output file %s.",out_file);
  tmp.inczero=include_zeroes;
  uint32_t start;
  uint32_t stop;
  float cvg;
  //Now we generate the bw info
  int chck = 0;
  int i=0;
	for(i=0;i<no_of_regions;i++){
	  //old last stop != end of cher and szeroes included then we add a final section of zero to the end of the contig){
	  chck = process_bam_region(input_file, pileup_func, &tmp, filter,  our_region_list[i], reference);
	  check(chck==1,"Error parsing bam region.");
	  start =  tmp.lstart;
    stop = tmp.lpos+1;
    cvg = tmp.lcoverage;
    if(start>0) bwAddIntervals(tmp.bwout,&tmp.head->target_name[tmp.ltid],&start,&stop,&cvg,single);
	  if(include_zeroes == 1){
	    char *contig = get_contig_from_region(our_region_list[i]);
	    uint32_t len = getContigLength(contig,chromList);
	    check(len != -1,"Error fetching length of contig %s.",contig);
      if(strcmp(contig,last_contig) != 0 && tmp.lstart != len){
        start = stop;
        if(stop==1) start = 0;
        stop = len;
        float zero = 0;
        bwAddIntervals(tmp.bwout,&contig,&start,&stop,&zero,single);
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
  for(clean=0; clean<sq_lines; clean++){
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
  return -1;
}
