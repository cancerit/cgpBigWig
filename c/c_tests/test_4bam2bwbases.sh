#!/bin/bash

########## LICENSE ##########
# Copyright (c) 2016-2018 Genome Research Ltd.
#
# Author: Cancer Genome Project cgphelp@sanger.ac.uk
#
# This file is part of cgpBigWig.
#
# cgpBigWig is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#    1. The usage of a range of years within a copyright statement contained within
#    this distribution should be interpreted as being equivalent to a list of years
#    including the first and last year specified and all consecutive years between
#    them. For example, a copyright statement that reads ‘Copyright (c) 2005, 2007-
#    2009, 2011-2012’ should be interpreted as being identical to a statement that
#    reads ‘Copyright (c) 2005, 2007, 2008, 2009, 2011, 2012’ and a copyright
#    statement that reads ‘Copyright (c) 2005-2012’ should be interpreted as being
#    identical to a statement that reads ‘Copyright (c) 2005, 2006, 2007, 2008,
#    2009, 2010, 2011, 2012’."
#
###########################

function error_exit
{
  echo "$1"
  echo "------"
  exit 1;
}

../bin/bam2bwbases -i ../test_data/volvox-sorted.bam -o ../test_data/tmp.bases.bw -F3844;
if [ "$?" != "0" ];
then
  rm -f ../test_data/*tmp.bases.bw;
  error_exit "ERROR in "$0": Running bam2bwbases"
fi

rm -f ../test_data/*tmp.bases.bw;

../bin/bam2bwbases -i ../test_data/volvox-sorted.bam -o ../test_data/tmp.bases.bw -F3844 -c ctgA:69-89;
if [ "$?" != "0" ];
then
  rm -f ../test_data/*tmp.bases.bw;
  error_exit "ERROR in "$0": Running bam2bwbases for region"
fi

#Check a region and check against the expected files
bases=("A" "C" "G" "T")

for b in ${bases[@]}
do
  ../bin/bwcat -i ../test_data/$b.expected.bases.bw > ../test_data/$b.bases.exp.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.expected.bases.bw > ../test_data/$b.bases.exp.bed";
  fi

  ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed";
  fi

  diff ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    rm -f ../test_data/*tmp.bases.bw;
    rm -f ../test_data/*.bases.got.bed;
    rm -f ../test_data/*.bases.exp.bed;
    error_exit "ERROR in "$0" running diff for bases ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed : Region bed file comparisons don't match";
  fi
done

rm -f ../test_data/*tmp.bases.bw;
rm -f ../test_data/*.bases.got.bed;
rm -f ../test_data/*.bases.exp.bed;

#Test without overlap
../bin/bam2bwbases -i ../test_data/TEST_wsig_overlap.bam -o ../test_data/tmp.bases.bw -F3844;
if [ "$?" != "0" ];
then
  rm -f ../test_data/*tmp.bases.bw;
  error_exit "ERROR in "$0": Running bam2bwbases"
fi

for b in ${bases[@]}
do
  ../bin/bwcat -i ../test_data/$b.TEST_wsig_overlap_bam2bwbases_no_overlap_expected.bw > ../test_data/$b.bases.exp.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.TEST_wsig_overlap_bam2bwbases_no_overlap_expected.bw > ../test_data/$b.bases.exp.bed";
  fi

  ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed";
  fi

  diff ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    rm -f ../test_data/*tmp.bases.bw;
    rm -f ../test_data/*.bases.got.bed;
    rm -f ../test_data/*.bases.exp.bed;
    error_exit "ERROR in "$0" running diff for bases ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed : Region bed file comparisons don't match";
  fi
done


rm -f ../test_data/*tmp.bases.bw;
rm -f ../test_data/*.bases.got.bed;
rm -f ../test_data/*.bases.exp.bed;


#Test with overlap
../bin/bam2bwbases -i ../test_data/TEST_wsig_overlap.bam -a -o ../test_data/tmp.bases.bw -F3844;
if [ "$?" != "0" ];
then
  rm -f ../test_data/*tmp.bases.bw;
  error_exit "ERROR in "$0": Running bam2bwbases"
fi

for b in ${bases[@]}
do
  ../bin/bwcat -i ../test_data/$b.TEST_wsig_overlap_bam2bwbases_with_overlap_expected.bw > ../test_data/$b.bases.exp.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.TEST_wsig_overlap_bam2bwbases_with_overlap_expected.bw > ../test_data/$b.bases.exp.bed";
  fi

  ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    error_exit "ERROR running ../bin/bwcat -i ../test_data/$b.tmp.bases.bw > ../test_data/$b.bases.got.bed";
  fi

  diff ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed;
  if  [ "$?" != "0" ];
  then
    rm -f ../test_data/*tmp.bases.bw;
    rm -f ../test_data/*.bases.got.bed;
    rm -f ../test_data/*.bases.exp.bed;
    error_exit "ERROR in "$0" running diff for bases ../test_data/$b.bases.exp.bed ../test_data/$b.bases.got.bed : Region bed file comparisons don't match";
  fi
done

rm -f ../test_data/*tmp.bases.bw;
rm -f ../test_data/*.bases.got.bed;
rm -f ../test_data/*.bases.exp.bed;
