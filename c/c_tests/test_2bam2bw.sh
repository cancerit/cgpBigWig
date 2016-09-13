#!/bin/bash

../bin/bam2bw -i ../test_data/volvox-sorted.bam -o ../test_data/tmp.bw;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Running bam2bw"
	echo "------"
	rm -f ../test_data/tmp.bw
  exit 1
fi

../bin/bwcat -i ../test_data/tmp.bw > ../test_data/tmp.out
if [ "$?" != "0" ];
then
  echo "ERROR running ../bin/bwcat -i ../test_data/tmp.bw > ../test_data/tmp.out"
  exit 1;
fi
../bin/bwcat -i ../test_data/volvox-sorted.coverage.expected.bw > ../test_data/expected.tmp.out
if [ "$?" != "0" ];
then
  echo "ERROR running ../bin/bwcat -i ../test_data/volvox-sorted.coverage.expected.bw > ../test_data/expected.tmp.out"
  exit 1;
fi

diff ../test_data/tmp.out ../test_data/expected.tmp.out
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Total file comparisons don't match"
	echo "------"
	rm -f ../test_data/tmp.out ../test_data/expected.tmp.out
  exit 1
fi

rm -f ../test_data/tmp.out ../test_data/expected.tmp.out ../test_data/tmp.bw