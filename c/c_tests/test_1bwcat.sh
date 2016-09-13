#!/bin/bash

../bin/bwcat -i ../test_data/test.bw > ../test_data/tmp.out;
if [ "$?" != "0" ];
then
  echo "ERROR running ../bin/bwcat -i ../test_data/test.bw > ../test_data/tmp.out"
  exit 1;
fi

diff ../test_data/tmp.out ../test_data/test_bw_out_expected.txt;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Total file comparisons don't match"
	echo "------"
	rm ../test_data/tmp.out
  exit 1
fi

../bin/bwcat -i ../test_data/test.bw -r 1:1006-1010 > ../test_data/tmp.out;
if [ "$?" != "0" ];
then
  echo "ERROR running ../bin/bwcat -i ../test_data/test.bw -r 1:1006-1010 > ../test_data/tmp.out"
  exit 1;
fi

diff ../test_data/tmp.out ../test_data/test_bw_out_expected_region.txt;
if  [ "$?" != "0" ];
then
  echo "ERROR in "$0": Single region file comparisons don't match"
	echo "------"
	rm ../test_data/tmp.out
  exit 1
fi

../bin/bwcat -i ../test_data/test.bw -r 1:1005-1010 > ../test_data/tmp.out;
if [ "$?" != "0" ];
then
  echo "ERROR running ../bin/bwcat -i ../test_data/test.bw -r 1:1005-1010 > ../test_data/tmp.out"
  exit 1;
fi

diff ../test_data/tmp.out ../test_data/test_bw_out_expected_2region.txt;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Two region file comparisons don't match"
	echo "------"
	rm ../test_data/tmp.out
  exit 1
fi

rm ../test_data/tmp.out