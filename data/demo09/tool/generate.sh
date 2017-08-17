#!/bin/bash

echo ============ GENERATE IMAGES ==============
echo

for ZIP in `ls prepare*.sh`
do
	bash $ZIP
done

echo ============ SORT IMAGES ==============
echo
bash sort_file.sh

echo ============ GENERATE MAPS ==============
echo
bash create_map.sh

rm *.zip
