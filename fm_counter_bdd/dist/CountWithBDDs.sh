#!/bin/bash

OUTPUT_FILE="results_with_bdds.csv"

for file in $(ls ../../benchmarks/industrial/); do
	timeout 86400s java -Xmx8000m -jar ./Counter.jar -m ../../../examples/SPLC/$file -o $OUTPUT_FILE | echo -e "../../benchmarks/industrial/${file};timeout;timeout;timeout" >> $OUTPUT_FILE
done
