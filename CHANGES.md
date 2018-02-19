# CHANGES

## NEXT

- Update to HTSlib 1.5 (for consistency across tools)
- Update to libBigWig 0.4.2
- Some README.md and INSTALL.md updates
 - Thanks go to [jsmedmar](https://github.com/jsmedmar) for documentation contributions

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
