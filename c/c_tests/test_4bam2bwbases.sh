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