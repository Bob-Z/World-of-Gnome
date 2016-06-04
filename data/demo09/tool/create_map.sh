#!/bin/bash

for NUM in `ls -d ? | sort`;do
	MAP_NAME=C$NUM
	echo "attribute = {" > $MAP_NAME
        echo "  x = {"	>> $MAP_NAME
        echo "        current = $NUM"	>> $MAP_NAME
	echo "      }"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
	echo "width = 10"	>> $MAP_NAME
	echo "height = 10"	>> $MAP_NAME
	echo "tile_width = 64"	>> $MAP_NAME
	echo "tile_height = 64"	>> $MAP_NAME
	echo "warp_x = 0"	>> $MAP_NAME
	echo "warp_y = 0"	>> $MAP_NAME
	echo "layer0 = {"	>> $MAP_NAME
	echo "        set = ["	>> $MAP_NAME

	FILE=`ls $NUM/WC_*_marquee.zip`
	for f in $FILE;do
		echo "\"character/$f\"," >> $MAP_NAME
	done
	#last coma
	echo "\"\"" >> $MAP_NAME

	echo "        ]"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
done
