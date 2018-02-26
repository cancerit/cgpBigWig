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

../bin/bg2bw -i ../test_data/volvox-sorted.coverage.expected.bed -o ../test_data/bg2bw.got.bw -c ../test_data/volvox.fa.fai;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Running bw2bg"
	echo "------"
	rm -f ../test_data/bg2bw.got.bw
  exit 1
fi

../bin/bwcat -i ../test_data/bg2bw.got.bw >  ../test_data/bg2bw.got.bed;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Running bwcat"
	echo "------"
	rm -f ../test_data/bg2bw.got.bw ../test_data/bg2bw.got.bed
  exit 1
fi

../bin/bwcat -i ../test_data/volvox-sorted.coverage.expected.bw > ../test_data/volvox-sorted.coverage.expected.out;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Running bwcat"
	echo "------"
	rm -f ../test_data/bg2bw.got.bw ../test_data/bg2bw.got.bed ../test_data/volvox-sorted.coverage.expected.out
  exit 1
fi



diff ../test_data/bg2bw.got.bed ../test_data/volvox-sorted.coverage.expected.out;
if [ "$?" != "0" ];
then
  echo "ERROR in "$0": Total file comparisons don't match"
	echo "------"
	rm -f ../test_data/bg2bw.got.bed ../test_data/bg2bw.got.bw ../test_data/volvox-sorted.coverage.expected.out
  exit 1
fi

rm -f ../test_data/bg2bw.got.bed ../test_data/bg2bw.got.bw ../test_data/volvox-sorted.coverage.expected.out