#!/bin/bash

MAP_QTY=`ls -d -1 ? | wc -l`
echo Generating $MAP_QTY maps

for NUM in `ls -d ? | sort`;do
	MAP_NAME=C$NUM

	FILE_QTY=`ls -1 $NUM/WC_*_marquee.zip | wc -l`
	echo $FILE_QTY files in map $NUM
	let "WIDTH=($FILE_QTY+1)/2"

	FILE=`ls $NUM/WC_*_marquee.zip`

	echo "attribute = {" > $MAP_NAME
        echo "  x = {"	>> $MAP_NAME
        echo "        current = $NUM"	>> $MAP_NAME
	echo "      }"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
	echo "width = $WIDTH"	>> $MAP_NAME
	echo "height = 3"	>> $MAP_NAME
	echo "tile_width = 64"	>> $MAP_NAME
	echo "tile_height = 64"	>> $MAP_NAME
	echo "warp_x = 0"	>> $MAP_NAME
	echo "warp_y = 0"	>> $MAP_NAME
	echo "layer0 = {"	>> $MAP_NAME
	echo "        set = ["	>> $MAP_NAME
	CURRENT_TILE=0
	for f in $FILE;do
		if [ "$CURRENT_TILE" = "$WIDTH" ];then
			for i in `seq 1 $WIDTH`;do
				echo "\"\"," >> $MAP_NAME
			done
		fi
		echo "\"character/$f\"," >> $MAP_NAME
		let "CURRENT_TILE=$CURRENT_TILE+1"
	done
	#last coma
	echo "\"\"" >> $MAP_NAME

	echo "        ]"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
done
