#!/bin/bash

function error_exit
{
  echo "$1"
  echo "------"
  exit 1;
}

../bin/bam2bedgraph -i ../test_data/volvox-sorted.bam -o ../test_data/tmp.bed;
if [ "$?" != "0" ];
then
  rm -f ../test_data/tmp.bed
  error_exit "ERROR in "$0": Running bam2bedgraph"
fi

diff ../test_data/tmp.bed ../test_data/volvox-sorted.coverage.expected.bed;
if [ "$?" != "0" ];
then
  rm -f ../test_data/tmp.bed;
  error_exit "ERROR in "$0" running ../test_data/tmp.bed ../test_data/volvox-sorted.coverage.expected.bed: Total bed file comparisons don't match";
fi

rm -f ../test_data/tmp.bed