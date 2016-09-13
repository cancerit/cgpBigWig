#!/bin/bash

########## LICENSE ##########
# Copyright (c) 2016 Genome Research Ltd.
#
# Author: Cancer Genome Project cgpit@sanger.ac.uk
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