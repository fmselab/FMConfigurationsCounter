#!/bin/bash

OUTPUT_FILE="results_with_bdds.csv"

for file in $(ls ../../../examples/SPLC/); do
	timeout 86400s java -Xmx8000m -jar ./Counter.jar -m ../../../examples/SPLC/$file -o $OUTPUT_FILE | echo -e "../../../examples/SPLC/${file};timeout;timeout;timeout" >> $OUTPUT_FILE
done
