echo "Running unit tests:"

for i in c_tests/*_tests
do
	if test -f $i
	then
		if $VALGRIND ./$i 2>> c_tests/tests_log
		then
			echo $i PASS
		else
			echo "ERROR in test $i: here's tests/tests_log"
			echo "------"
			tail c_tests/tests_log
			exit 1
		fi
	fi
done

echo "Running script tests:"

for j in c_tests/test_*.sh
do
  if ./$j 2>> c_tests/tests_log
   then echo $j PASS
  else
    echo "ERROR in "$j": here's c_tests/tests_log"
    echo "------"
    tail c_tests/tests_log
    exit 1
  fi
done
echo ""
