end=$1
for i in `seq 0 $((end-1))`
do
	. test_param_dump.sh $i
done
