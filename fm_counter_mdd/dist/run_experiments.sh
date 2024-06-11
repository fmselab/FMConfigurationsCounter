#!/bin/bash

OUTPUT_FILE="experiments.csv"
CTC_REDUCTION=(1 2 5 10 20 50)
AND_REDUCTION=(0 1 2 5 10 20 50)

for file in $(ls ../../benchmarks/synthetic/); do
	for r in ${CTC_REDUCTION[@]}; do
		for a in ${AND_REDUCTION[@]}; do
			if [ a == 0 ]; then
				timeout 3600s ./FMBuilderExperimenter --m ../../benchmarks/synthetic/$file --r $r --o $OUTPUT_FILE --dr || echo -e "../../benchmarks/synthetic/${file};timeout;timeout;${r};0;${a};1;timeout;timeout" >> $OUTPUT_FILE
				timeout 3600s ./FMBuilderExperimenter --m ../../benchmarks/synthetic/$file --r $r --o $OUTPUT_FILE || echo -e "../../benchmarks/synthetic/${file};timeout;timeout;${r};0;${a};0;timeout;timeout" >> $OUTPUT_FILE
			else
				timeout 3600s ./FMBuilderExperimenter --m ../../benchmarks/synthetic/$file --r $r --o $OUTPUT_FILE --dr --mergeAnd --nMergeAnd $a  || echo -e "../../benchmarks/synthetic/${file};timeout;timeout;${r};1;${a};1;timeout;timeout" >> $OUTPUT_FILE
				timeout 3600s ./FMBuilderExperimenter --m ../../benchmarks/synthetic/$file --r $r --o $OUTPUT_FILE --mergeAnd --nMergeAnd $a || echo -e "../../benchmarks/synthetic/${file};timeout;timeout;${r};1;${a};0;timeout;timeout" >> $OUTPUT_FILE
			fi
		done
	done
done
