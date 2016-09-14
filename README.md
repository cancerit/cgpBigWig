cgpBigWig
==============

Package of C scripts for generation of [BigWig](https://genome.ucsc.edu/goldenpath/help/bigWig.html) coverage files

---

###Dependencies/Install

Some of the code included in this package has dependencies on external packages:

 * [libBigWig](https://github.com/dpryan79/libBigWig)
 * [htslib](https://github.com/samtools/htslib)

Please see the respective licence for each before use.

Please use `setup.sh` to install the dependencies.  Please be aware that this expects basic C
compilation libraries and tools to be available, most are listed in [`INSTALL`](INSTALL.md).

---

### Programs

[bwcat](#bwcat) - Read the contents of a bigwig (.bw) file

[bwjoin](#bwjoin) - Concatenate bigwig files together

[bam2bw](#bam2bw) - Generate bigwig (.bw) coverage file from bam

[bam2bwbases](#bam2bwbases) - Generate bigwig (.bw) proportion file of each base at a position from bam

[bam2bedgraph](#bam2bedgraph) - Generate a coverage bedgraph from bam

##### bwcat
Read the contents of a bw file
```
Usage: bwcat -i input-path

-i  --input-path [file]    Path to the input bigwig file

Optional:
-r  --region [string]      Region or bw file to print to screen format. NB start should be 0 based: (contig:start-stop)
-n  --include-na           Include NA regions in output
-o  --output [file]        File to write bw output to [default: stdout]

Other:
-h --help                  Display this usage information.
-v --version               Prints the version number.
```

##### bwjoin
Concatenate bw files together
```
Usage: bwjoin -f genome.fai -o output.bw

-p  --input-path [dir]                           Path to the input bigwig files named [path]/<contig_name>.bw
-f  --fasta-index [file]                         Fasta index file (.fai)
-o  --outfile [file]                             Path to the output .bw file produced. [default:'concatenated.bw']
-g  --ignore-contigs [comma separated list]      Comma separated list of contigs to ignore.

Other:
-h --help      Display this usage information.
-v --version   Prints the version number.
```

##### bam2bw
Generate bw coverage file from bam
```
Usage: bam2bw -i input.[b|cr]am -o output.bw
bam2bw can be used to generate a bw file of coverage from a [cr|b]am file.

-i  --input [file]                                Path to the input [b|cr]am file.
-F  --filter [int]                                SAM flags to filter. [default: 4]
-o  --outfile [file]                              Path to the output .bw file produced. [default:'output.bam.bw']

Optional:
-c  --region [file]                               A samtools style region (contig:start-stop) or a bed file of regions over which to produce the bigwig file
-z  --include-zeroes                              Include zero coverage regions as additional entries to the bw file
-r  --reference [file]                            Path to reference genome.fa file (required for cram if ref_path cannot be resolved)

Other:
-h  --help                                        Display this usage information.
-v  --version                                     Prints the version number.
```

##### bam2bwbases
Generate bw proportion file of each base at a position from bam
```
Usage: bam2bwbases -i input.[b|cr]am -o output.bw
bam2bwbases can be used to generate four bw files of per base proportions.

-i  --input [file]                                Path to the input [b|cr]am file.
-F  --filter [int]                                SAM flags to filter. [default: 4]
-o  --outfile [file]                              Path to the output .bw file produced. Per base results wiillbe output as four bw files [ACGT].outputname.bw [default:'(null)']

Optional:
-c  --region [file]                               A samtools style region (contig:start-stop) or a bed file of regions over which to produce the bigwig file
-r  --reference [file]                            Path to reference genome.fa file (required for cram if ref_path cannot be resolved)

Other:
-h  --help                                        Display this usage information.
-v  --version                                     Prints the version number.
```

##### bam2bedgraph
Generate a coverage bedgraph from bam
```
Usage: bam2bedgraph -i input.[cr|b]am -o file [-r region] [-h] [-v]

Create a BEDGraph file of genomic coverage. BAM file must be sorted.
-i --input     Path to bam/cram input file. [default: stdin]
-o --output    File path for output. [default: stdout]

Optional:
-r --region    Region in bam to access.
-f --filter    Ignore reads with the filter flags [int].
Other:
-h --help      Display this usage information.
-v --version   Prints the version number.
```

