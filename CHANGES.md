# CHANGES

## 1.4.0

* Move cwl and examples into core cgpBigWig respository

## 1.3.0

* Update to htslib 1.10.2
  * Required for PCAP-core to update to samtools 1.10 for new features
* Update `setup.sh` to exec `build/*` scripts to reduce chance of divergence
* Update Docker image to Ubuntu Focal (20.04) base.
* Fix #33 (typo)

## 1.2.0

Add bam2bw -S/--scale-log10 to enable simple RPKM rescaling.
Thanks to @shouldsee for these modifications.

## 1.1.0

Add docker build to allow internal large CI system to process. dockstore-cgpbigwig will
be modified to base from here and only contain additional helpers and CWL.

## 1.0.4

* Add checks to sam_iter_next to ensure result is checked for errors

## 1.0.3

### Behaviour change

**When the proper pair filter flag is used, this code now checks that the paired-end orientation is also used.**
**This will mean that mate-pair orientation (F/F or R/R) will be rejected**

* Where a proper pair filter is used, now check for the correct paired-end orientation of F/R.
* If this is not met the read is ignored
* Added a -f filter of flags to **include** alongside the existing -F for exclude as per samtools style filtering for `bam2bw`, `bam2bwbases` and `bam2bedgraph`
* **NB this flag was used as the exclude in `bam2bedgraph`. The flags have now been switched.**

## 1.0.2

* Correct region parsing in bam2bw to cope woth GRCh38 contigs

## 1.0.1

* Fix bam2bw. Single region string and -z caused header issues.

## 1.0.0

* First full release
* Add overlapping reads support to `bam2bw`, `bam2bwbases` and `bam2bedgraph` via commandline flag `--overlap` `-a`
  * If the same readname is encountered twice at a position it is considred an overlapping read pair.
  * Where the same base [ACGT] was encountered on each of the reads it will only be counted once. If a different base was encountered then the coverage count is incremented once for each base.
* Updated License headers with new email address

## 0.5.0

* Update to HTSlib 1.5 (for consistency across tools)
* Update to libBigWig 0.4.2
* Some README.md and INSTALL.md updates
  * Thanks go to [jsmedmar](https://github.com/jsmedmar) for documentation contributions

## 0.4.4

* Correct parseRegionString return type. Fixes [#11](https://github.com/cancerit/cgpBigWig/issues/11).
* Above change also fixes [#12](https://github.com/cancerit/cgpBigWig/issues/12).

## 0.4.3

* Updated libBigWig to 0.3.4 - fixes #10

## 0.4.2

* Modified parsing of regions to enable names with : in (i.e. GRCh38)

## 0.4.1

* Updated libBigWig to 0.3.1 - fixes issue with overflow in very large zoom levels

## 0.4.0

* Added detectExtremeDepth code. Replaces functionality in PCAP-core detectExtremeDepth.pl

## 0.3.1

* Changes HTSlib to 1.3.2 (bringing all stack in line)
* Changed libBigWig archive get to use release rather than commit id (actually the same but harder to tell which release it is)

## 0.3.0

* Changes to setup.sh to permit OSX installation

## 0.2.1
* Corrections to logic


## 0.2.0

* Added bedgraph to bw functionality via bg2bw

## 0.1.0

* First release
